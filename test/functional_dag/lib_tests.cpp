#include "functional_dag/lib_utils.h"
#include <catch2/catch_test_macros.hpp>


// #include <random>

const std::string expected_json = \
"{\"nodes\":{\"test viz\":{\"guid\":1828100,\"parents\":{\"\":\"test lib\"}},\"sources\":{\"test lib\":{\"guid\":8179941,\"opts\":[{\"id\":111111,\"val\":\"localhost\"},{\"id\":222222,\"val\":1234}]}}}\n";

class src_mock : fn_dag::module_source {
  src_mock() {}
  ~src_mock() {}
  DLTensor *update() {return nullptr;}
};

class node_mock : fn_dag::module_transmit {
  node_mock() {}
  ~node_mock() {}
  DLTensor *update(const DLTensor *) {return nullptr;}
};


TEST_CASE( "Serializes JSON", "[libs.json_serialize]" ) {
  // Create a fake library and add it to the libs 
  std::vector<fn_dag::library_spec> dag;
  fn_dag::library_spec lib1;
  lib1.name = "test lib";
  lib1.lib_guid = 8179941;
  lib1.parent_name = "";
  lib1.is_source = true;
  
  fn_dag::construction_option url;
  fn_dag::construction_option port;

  url.type = fn_dag::OPTION_TYPE::STRING;
  url.serial_id = 111111;
  url.value.string_value = "localhost";

  port.type = fn_dag::OPTION_TYPE::INT;
  port.serial_id = 222222;
  port.value.int_value = 1234;
  
  lib1.instantiation_options.push_back(url);
  lib1.instantiation_options.push_back(port);

  // Create a second fake library and add it to the libs
  fn_dag::library_spec lib2;
  lib2.name = "test viz";
  lib2.lib_guid = 1828100;
  lib2.parent_name = "test lib";
  lib2.is_source = false;

  // // insert it to the graph
  dag.push_back(lib1);
  dag.push_back(lib2);
  // serialize out
  std::string json_out = fsys_serialize(&dag);
  std::cout << "json_out:\n" << json_out << std::endl;

  REQUIRE( expected_json == json_out );
}

TEST_CASE( "Deserializes JSON", "[libs.json_deserialize]" ) {
  std::unordered_map<uint32_t, fn_dag::instantiate_fn> library;
  fn_dag::instantiate_fn spec1;
  spec1 = [] (const fn_dag::lib_options *opts_read) {
    fn_dag::module *module_out = nullptr;
    for(auto option : *opts_read) {
      if(option.serial_id == 111111) {
        REQUIRE(option.type == fn_dag::OPTION_TYPE::STRING);
        REQUIRE(std::string(option.value.string_value) == "localhost");
      }
      if(option.serial_id == 222222) {
        REQUIRE(option.type == fn_dag::OPTION_TYPE::INT);
        REQUIRE(option.value.int_value == 1234);
      }
    }
    return std::shared_ptr<fn_dag::module>(module_out);
  };
  library.emplace(8179941, spec1);
  
  // std::shared_ptr<fn_dag::lib_specification> spec2;
  fn_dag::instantiate_fn spec2;
  // spec2 = std::shared_ptr<fn_dag::lib_specification>(new fn_dag::lib_specification());
  bool did_instantiate = false;
  // spec2->module_handle = 
  spec2 = [&] (const fn_dag::lib_options *opts_read) {
    fn_dag::module *module_out = nullptr;
    REQUIRE(opts_read->size() == 0);
    did_instantiate = true;
    return std::shared_ptr<fn_dag::module>(module_out);
  };
  library.emplace(1828100, spec2);

  // deserialize out
  auto manager = fsys_deserialize(expected_json, library);
  delete manager;
  REQUIRE( !did_instantiate );
}
