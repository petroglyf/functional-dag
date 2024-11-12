#include <catch2/catch_test_macros.hpp>
#include <functional_dag/fn_dag_interface.hpp>
#include "functional_dag/filter_sys.hpp"

#include <chrono>
#include <thread>
#include <random>
#include <sstream>

TEST_CASE( "Fill an array in order", "[dag.single_thread]" ) {
  int array[] = {0,0,0,0,0};
  int ran_times = 0;

  fn_dag::dag_manager<uint64_t> manager;
  manager.run_single_threaded(true);
  

  for(uint64_t i = 0;i < 5;i++) {
    std::function<int *()> fn = [&array, i, &ran_times]() {
      *(array+i) = i+1;
      ran_times++;
      int *int_out = new int;
      *int_out = *(array+i);
      return int_out;
    };
    
    manager.add_dag(i, fn_dag::fn_source(fn), false);
  }

  for(auto dag : manager.m_all_dags) {
    dag->push_once();
  }

  REQUIRE( array[0] == 1 );
  REQUIRE( array[1] == 2 );
  REQUIRE( array[2] == 3 );
  REQUIRE( array[3] == 4 );
  REQUIRE( array[4] == 5 );
  REQUIRE( ran_times == 5 );
}

TEST_CASE( "Fill an array out of order", "[dag.multithread]" ) {
  int array_out[] = {0,0,0,0,0};
  int array_run_order[] = {0,0,0,0,0};
  int run_order = 1;

  fn_dag::dag_manager<uint64_t> manager;

  for(uint64_t i = 0;i < 5;i++) {
    std::function<int *()> fn = [&array_out, i, &run_order, &array_run_order]() {
      if( *(array_run_order+i) == 0 ) {
        int hundreds_of_ms = rand() % 36;
        int ms = hundreds_of_ms*100;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        *(array_out+i) = i+1;
        *(array_run_order+i) = run_order++;
      }
      return nullptr;
    };
    
    manager.add_dag(i, fn_dag::fn_source(fn), false);
  }

  manager.print_all_dags();
  for(auto dag : manager.m_all_dags)
    dag->push_once();
  
  std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  manager.stahp();

  REQUIRE( array_out[0] == 1 );
  REQUIRE( array_out[1] == 2 );
  REQUIRE( array_out[2] == 3 );
  REQUIRE( array_out[3] == 4 );
  REQUIRE( array_out[4] == 5 );
  
  bool in_order = true;
  for(int j = 0;j < 5;j++) {
    in_order = in_order && array_run_order[j+1] == j+1; 
  }

  REQUIRE_FALSE(in_order);
}

TEST_CASE( "Use a fanout, check all 5 received", "[dag.fanout]" ) {
  int array[] = {0,0,0,0,0};
  int rand_int = -1;    
  fn_dag::dag_manager<int> manager;

  std::function<int *()> fn = [&rand_int]() {
    rand_int = rand() % 10;
    int *pass_int = new int;
    *pass_int = rand_int;
    return pass_int;
  };
  
  manager.add_dag(0, fn_dag::fn_source(fn), false);

  for(int i = 0;i < 5;i++) {
    std::function<int *(const int *)> fn_c = [&array, i](const int *rand_int_in) {
      *(array+i) = *rand_int_in;
      return nullptr;
    };
      
    manager.add_node(i+1, fn_dag::fn_call(fn_c), 0);
  }
  manager.print_all_dags();
  for(auto dag : manager.m_all_dags) {
    dag->push_once();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  manager.stahp();

  REQUIRE( array[0] == rand_int );
  REQUIRE( array[1] == rand_int );
  REQUIRE( array[2] == rand_int );
  REQUIRE( array[3] == rand_int );
  REQUIRE( array[4] == rand_int );
}

TEST_CASE( "Simple accumulate", "[dag.accumulate]" ) {
  fn_dag::dag_manager<int> manager;

  std::function<int *()> fn = []() {
    int *pass_int = new int;
    *pass_int = 1;
    return pass_int;
  };
  
  manager.add_dag(0, fn_dag::fn_source(fn), false);

  for(int i = 0;i < 9;i++) {
    std::function<int *(const int *)> fn_c = [](const int *int_in) {
      int *pass_int = new int;
      *pass_int = *int_in + 1;        
      return pass_int;
    };

    manager.add_node(i+1, fn_dag::fn_call(fn_c), i);
  }

  int final_value;
  std::function<int *(const int *)> fn_last = [&final_value](const int *int_in) {
    final_value = *int_in;
    return nullptr;
  };
  manager.add_node(10, fn_dag::fn_call(fn_last), 9);
  manager.print_all_dags();
  for(auto dag : manager.m_all_dags)
    dag->push_once();
  
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  manager.stahp();

  REQUIRE( final_value == 10 );
}

TEST_CASE( "Print the dag and check results", "[dag.print]" ) {
  fn_dag::dag_manager<int> manager;

  std::function<int *()> fn = []() {
    int *pass_int = new int;
    *pass_int = 1;
    return pass_int;
  };
  
  manager.add_dag(0, fn_dag::fn_source(fn), false);

  for(int i = 0;i < 9;i++) {
    std::function<int *(const int *)> fn_c = [](const int *int_in) {
      int *pass_int = new int;
      *pass_int = *int_in + 1;        
      return pass_int;
    };

    manager.add_node(i+1, fn_dag::fn_call(fn_c), i);
  }
  std::stringstream output_stream;
  manager.set_logging_stream(&output_stream);
  manager.print_all_dags();
  std::string final_string = output_stream.str();
  auto num_newlines = std::count(final_string.begin(), final_string.end(), '\n');
  
  // 10 nodes, 2 header+footer, 1 extra 
  REQUIRE( num_newlines == 10+2+1 );
}