#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <functional>

#include "functional_dag/dag_interface.hpp"
#include "functional_dag/error_codes.h"
#include "functional_dag/filter_sys.hpp"
#include "functional_dag/guid_impl.hpp"
#include "functional_dag/lib_spec_generated.h"
#include "functional_dag/libutils.h"

using namespace fn_dag;
using namespace std;

library_spec get_library_details() {
  const string_view src_guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  const string_view viz_guid_str("e5f4e68b-549a-4796-b118-479ecc0a370b");
  const string_view lib_guid_str("54c8441d-9973-4d2e-a05a-5b7ddd9bf819");
  auto src_guid_ret = GUID<node_prop_spec>::from_uuid(src_guid_str);
  auto viz_guid_ret = GUID<node_prop_spec>::from_uuid(viz_guid_str);
  auto lib_guid_ret = GUID<library>::from_uuid(lib_guid_str);

  GUID<node_prop_spec> src_guid = src_guid_ret.value();
  GUID<node_prop_spec> viz_guid = viz_guid_ret.value();
  GUID<library> lib_guid = lib_guid_ret.value();

  auto test_string = string("test_string");
  option_spec test_option_spec{.type = fn_dag::OPTION_TYPE_STRING,
                               .name = test_string,
                               .option_prompt = test_string,
                               .short_description = test_string};

  node_prop_spec node_prop_spec_src{.guid = src_guid,
                                    .name = "test_source",
                                    .description = "test_source details",
                                    .module_type = NODE_TYPE::NODE_TYPE_SOURCE,
                                    .construction_types = {test_option_spec}};

  node_prop_spec node_prop_spec_viz{.guid = viz_guid,
                                    .name = "test_viz",
                                    .description = "test_viz details",
                                    .module_type = NODE_TYPE::NODE_TYPE_FILTER,
                                    .construction_types = {test_option_spec}};

  return {.guid = lib_guid,
          .available_nodes = {node_prop_spec_src, node_prop_spec_viz}};
}

class test_src : public dag_source<int> {
 public:
  unique_ptr<int> update() { return std::make_unique<int>(0); }
};

class test_flt : public dag_node<int, float> {
 public:
  unique_ptr<float> update(const int *const y) {
    assert(y != nullptr && *y == 0);
    return std::make_unique<float>(0.0f);
  }
};

bool construct_node(dag_manager<string> &manager, const node_spec &spec) {
  const string_view src_guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
  const string_view viz_guid_str("e5f4e68b-549a-4796-b118-479ecc0a370b");
  auto src_guid_ret = GUID<node_spec>::from_uuid(src_guid_str);
  auto viz_guid_ret = GUID<node_spec>::from_uuid(viz_guid_str);

  GUID<node_spec> src_guid = src_guid_ret.value();
  GUID<node_spec> viz_guid = viz_guid_ret.value();

  string test_string_value;
  for (auto option : *spec.options()) {
    if (option->value()->type() == fn_dag::OPTION_TYPE_STRING &&
        option->name()->string_view() == "test_string") {
      test_string_value += option->value()->string_value()->string_view();
    }
  }
  GUID<node_spec> spec_guid{*spec.target_id()};

  if (spec_guid == src_guid) {
    REQUIRE(manager.add_dag(spec.name()->str(), new test_src(), false));
    return true;
  } else if (spec_guid == viz_guid) {
    auto src_node_name = spec.wires()->Get(0)->value()->str();
    if (!manager.add_node(spec.name()->str(), new test_flt(), src_node_name))
      return false;
    return true;
  }
  return false;
}

class library_example : public library {
 public:
  library_example() : library() {
    const string_view src_guid_str("2253c551-dd08-4ff4-928d-9b1e8c586c14");
    const string_view viz_guid_str("e5f4e68b-549a-4796-b118-479ecc0a370b");

    auto src_guid_ret = GUID<node_spec>::from_uuid(src_guid_str);
    auto viz_guid_ret = GUID<node_spec>::from_uuid(viz_guid_str);

    GUID<node_spec> src_guid = src_guid_ret.value();
    GUID<node_spec> viz_guid = viz_guid_ret.value();

    m_constructors[src_guid] =
        function<construction_signature>(&construct_node);
    m_constructors[viz_guid] =
        function<construction_signature>(&construct_node);
  }
};

TEST_CASE("Deserializes JSON", "[libs.json_deserialize_success]") {
  string json_str =
      "{\
    nodes:\
    [\
        {\
            name: \"ex_node\",\
            target_id: {bits1: 16570122415097137046, bits2: 12761028291507926795},\
            wires: [{key: \"y\", value:\"ex_source\"}],\
            options: [{name: \"test_string\", value: {type: INT, int_value: 5}}]\
        },\
    ],\
    sources:\
    [\
        {\
            name : \"ex_source\",\
            target_id: {bits1 : 2473537575747866612, bits2 : 10560267256759610388},\
            wires : [],\
            options: [{name: \"cons_in\", value: {type: INT, int_value: 10}}]\
        }\
    ]\
    }";
  library_example library_ex;

  if (auto manager = library_ex.fsys_deserialize(json_str); manager) {
    auto real_manager = manager.value();
    REQUIRE(real_manager->manager_contains_id("ex_source"));
    REQUIRE(real_manager->manager_contains_id("ex_node"));
    delete real_manager;
  } else {
    REQUIRE(manager.has_value());
  }
}

TEST_CASE("Serializes JSON", "[libs.json_serialize_success]") {
  flatbuffers::FlatBufferBuilder builder(1024);
  GUID_vals vals(11, 44);
  vector<::flatbuffers::Offset<string_mapping>> wires;
  vector<::flatbuffers::Offset<construction_option>> options;
  auto node_name = builder.CreateString("test_node");
  auto option_name = builder.CreateString("test_option");
  auto src_string = builder.CreateString("src_string");
  auto target_str = builder.CreateString("target_string");
  auto opt_val_builder = option_valueBuilder(builder);
  opt_val_builder.add_type(fn_dag::OPTION_TYPE_INT);
  opt_val_builder.add_int_value(42);
  auto opt_val = opt_val_builder.Finish();
  auto const_opt_builder = construction_optionBuilder(builder);
  const_opt_builder.add_name(option_name);
  const_opt_builder.add_value(opt_val);
  auto const_opt = const_opt_builder.Finish();
  ::flatbuffers::Offset<construction_option> oarr[] = {const_opt};
  auto opt_arr = builder.CreateVector(oarr, 1);
  auto str_map_builder = string_mappingBuilder(builder);
  str_map_builder.add_key(src_string);
  str_map_builder.add_value(target_str);
  auto str_map = str_map_builder.Finish();
  ::flatbuffers::Offset<string_mapping> smarr[] = {str_map};
  auto sm_arr = builder.CreateVector(smarr, 1);
  auto node_spec_builder = node_specBuilder(builder);
  node_spec_builder.add_name(node_name);
  node_spec_builder.add_options(opt_arr);
  node_spec_builder.add_wires(sm_arr);
  node_spec_builder.add_target_id(&vals);
  auto node_spec = node_spec_builder.Finish();
  ::flatbuffers::Offset<fn_dag::node_spec> nsarr[] = {node_spec};
  auto ns_arrs = builder.CreateVector(nsarr, 1);
  auto ns_arrt = builder.CreateVector(nsarr, 0);
  auto pipe_spec_builder = pipe_specBuilder(builder);
  pipe_spec_builder.add_nodes(ns_arrt);
  pipe_spec_builder.add_sources(ns_arrs);
  auto pipe_spec = pipe_spec_builder.Finish();
  builder.Finish(pipe_spec);
  uint8_t *data = builder.GetBufferPointer();
  if (auto json = fsys_serialize(data); json) {
    REQUIRE(json.value().size() > 0);
  }
}

TEST_CASE("Bad JSON", "[libs.json_bad_data]") {
  string empty_json_str = "";
  string bad_key =
      "{\
    nodes1:\
    [],\
    sources:\
    []\
  }";
  string bad_type =
      "{\
    nodes1:\
    \"\",\
    sources:\
    []\
  }";
  library_example library_ex;

  auto manager_empty = library_ex.fsys_deserialize(empty_json_str);
  auto manager_badkey = library_ex.fsys_deserialize(bad_key);
  auto manager_badtype = library_ex.fsys_deserialize(bad_type);

  REQUIRE(manager_empty.error() == fn_dag::JSON_PARSER_ERROR);
  REQUIRE(manager_badkey.error() == fn_dag::JSON_PARSER_ERROR);
  REQUIRE(manager_badtype.error() == fn_dag::JSON_PARSER_ERROR);
}
