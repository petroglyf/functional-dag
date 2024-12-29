#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <functional_dag/fn_dag_interface.hpp>
#include <memory>
#include <sstream>
#include <thread>

#include "functional_dag/dag_interface.hpp"
#include "functional_dag/filter_sys.hpp"

TEST_CASE("Fill an array in order", "[dag.single_thread]") {
  int array[] = {0, 0, 0, 0, 0};
  int ran_times = 0;

  fn_dag::dag_manager<uint16_t> manager;
  manager.run_single_threaded(true);

  for (uint16_t i = 0; i < 5; i++) {
    std::function<std::unique_ptr<int>()> fn = [&array, i, &ran_times]() {
      *(array + i) = i + 1;
      ran_times++;
      return std::make_unique<int>(*(array + i));
    };
    REQUIRE(manager.add_dag(i, fn_dag::fn_source<int>(fn), false));
  }

  for (auto dag : manager.m_all_dags) {
    dag->push_once();
  }

  REQUIRE(array[0] == 1);
  REQUIRE(array[1] == 2);
  REQUIRE(array[2] == 3);
  REQUIRE(array[3] == 4);
  REQUIRE(array[4] == 5);
  REQUIRE(ran_times == 5);
}

TEST_CASE("Fill an array out of order", "[dag.multithread]") {
  int array_out[] = {0, 0, 0, 0, 0};
  int array_run_order[] = {0, 0, 0, 0, 0};
  int run_order = 1;

  fn_dag::dag_manager<uint64_t> manager;

  for (uint64_t i = 0; i < 5; i++) {
    std::function<std::unique_ptr<int>()> fn = [&array_out, i, &run_order,
                                                &array_run_order]() {
      if (*(array_run_order + i) == 0) {
        int hundreds_of_ms = rand() % 36;
        int ms = hundreds_of_ms * 100;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        *(array_out + i) = i + 1;
        *(array_run_order + i) = run_order++;
      }
      return nullptr;
    };

    REQUIRE(manager.add_dag(i, fn_dag::fn_source<int>(fn), false));
  }

  manager.print_all_dags();
  for (auto dag : manager.m_all_dags) dag->push_once();

  std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  manager.stahp();

  REQUIRE(array_out[0] == 1);
  REQUIRE(array_out[1] == 2);
  REQUIRE(array_out[2] == 3);
  REQUIRE(array_out[3] == 4);
  REQUIRE(array_out[4] == 5);

  bool in_order = true;
  for (int j = 0; j < 5; j++) {
    in_order = in_order && array_run_order[j + 1] == j + 1;
  }

  REQUIRE_FALSE(in_order);
}

TEST_CASE("Use a fanout, check all 5 received", "[dag.fanout]") {
  int array[] = {0, 0, 0, 0, 0};
  int rand_int = -1;
  fn_dag::dag_manager<int> manager;

  std::function<std::unique_ptr<int>()> fn = [&rand_int]() {
    rand_int = rand() % 10;
    return std::make_unique<int>(rand_int);
  };

  REQUIRE(manager.add_dag(0, fn_dag::fn_source(fn), false));

  for (int i = 0; i < 5; i++) {
    std::function<std::unique_ptr<int>(const int *const)> fn_c =
        [&array, i](const int *const rand_int_in) {
          *(array + i) = *rand_int_in;
          return nullptr;
        };

    REQUIRE(manager.add_node(i + 1, fn_dag::fn_call(fn_c), 0));
  }
  REQUIRE(manager.m_all_dags.size() == 1);
  manager.print_all_dags();
  for (auto dag : manager.m_all_dags) {
    dag->push_once();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  manager.stahp();

  REQUIRE(array[0] == rand_int);
  REQUIRE(array[1] == rand_int);
  REQUIRE(array[2] == rand_int);
  REQUIRE(array[3] == rand_int);
  REQUIRE(array[4] == rand_int);
}

TEST_CASE("Simple accumulate", "[dag.accumulate]") {
  fn_dag::dag_manager<int> manager;

  std::function<std::unique_ptr<int>()> fn = []() {
    return std::make_unique<int>(1);
  };

  REQUIRE(manager.add_dag(0, fn_dag::fn_source(fn), false));

  for (int i = 0; i < 9; i++) {
    std::function<std::unique_ptr<int>(const int *const)> fn_c =
        [](const int *const int_in) {
          return std::make_unique<int>(*int_in + 1);
        };

    REQUIRE(manager.add_node(i + 1, fn_dag::fn_call(fn_c), i));
  }

  int final_value;
  std::function<std::unique_ptr<int>(const int *const)> fn_last =
      [&final_value](const int *const int_in) {
        final_value = *int_in;
        return nullptr;
      };
  REQUIRE(manager.add_node(10, fn_dag::fn_call(fn_last), 9));

  manager.print_all_dags();
  for (auto dag : manager.m_all_dags) dag->push_once();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  manager.stahp();

  REQUIRE(final_value == 10);
}

TEST_CASE("Print the dag and check results", "[dag.print]") {
  fn_dag::dag_manager<int> manager;

  std::function<std::unique_ptr<int>()> fn = []() {
    return std::make_unique<int>(1);
  };

  REQUIRE(manager.add_dag(0, fn_dag::fn_source(fn), false));

  for (int i = 0; i < 9; i++) {
    std::function<std::unique_ptr<int>(const int *const)> fn_c =
        [](const int *const int_in) {
          return std::make_unique<int>(*int_in + 1);
        };
    REQUIRE(manager.add_node(i + 1, fn_dag::fn_call(fn_c), i));
  }
  std::stringstream output_stream;
  manager.set_logging_stream(&output_stream);
  manager.print_all_dags();
  std::string final_string = output_stream.str();
  auto num_newlines =
      std::count(final_string.begin(), final_string.end(), '\n');

  // 10 nodes, 2 header+footer, 1 extra
  REQUIRE(num_newlines == 10 + 2 + 1);
}

TEST_CASE("Check that errors are thrown for null pointer nodes",
          "[dag.null_nodes]") {
  fn_dag::dag_manager<int> manager;

  auto result = manager.add_dag(0, (fn_dag::dag_source<int> *)nullptr, false);
  REQUIRE_FALSE(result);
  REQUIRE(result.error() == fn_dag::error_codes::NULL_PTR_ERROR);

  auto result2 = manager.add_node(1, (fn_dag::dag_node<int, int> *)nullptr, 0);
  REQUIRE_FALSE(result2);
  REQUIRE(result2.error() == fn_dag::error_codes::NULL_PTR_ERROR);
}

TEST_CASE("Check that errors are thrown for not found parents",
          "[dag.missing_parents]") {
  fn_dag::dag_manager<int> manager;

  std::function<std::unique_ptr<int>()> fn = []() {
    return std::make_unique<int>(1);
  };

  std::function<std::unique_ptr<int>(const int *const)> fn_node =
      [](const int *const) { return std::make_unique<int>(1); };

  REQUIRE(manager.add_dag(0, fn_dag::fn_source(fn), false));

  auto result = manager.add_node(1, (fn_dag::dag_node<int, int> *)nullptr, 6);
  REQUIRE_FALSE(result);
  REQUIRE(result.error() == fn_dag::error_codes::NULL_PTR_ERROR);

  auto result2 = manager.add_node(1, fn_dag::fn_call(fn_node), 6);
  REQUIRE_FALSE(result2);
  REQUIRE(result2.error() == fn_dag::error_codes::PARENT_NOT_FOUND);
}