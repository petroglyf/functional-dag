#include <stdlib.h> 
#include <dlfcn.h>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <filter_sys/dlpack.h>
#include <filter_sys/lib_utils.h>
#include "json/json.h"


static const fs::directory_entry library_path("./lib");

#ifdef __APPLE__
static const string dylib_suffix("dylib");
#elif __linux__
static const string dylib_suffix("so");
#endif

using namespace std;

shared_ptr< vector<fs::directory_entry> > get_all_available_libs() {
  shared_ptr< vector<fs::directory_entry> > all_files(new vector<fs::directory_entry>());

  if(library_path.exists()) {
    for (const auto & entry : fs::directory_iterator(library_path.path()))
      if(entry.path().string().ends_with(dylib_suffix))
        all_files->push_back(entry);
  } else
    cerr << "Could not find library directory: " << library_path.path().string() << endl;
  return all_files;
}

bool preflight_lib(const fs::path _lib_path) {
  
  void * const lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_FIRST);
  
  bool has_description = dlsym(lib_handle, "get_simple_description") != NULL;
  has_description = has_description && dlsym(lib_handle, "get_detailed_description") != NULL;
  
  bool has_guid = dlsym(lib_handle, "get_serial_guid") != NULL;
  bool has_name = dlsym(lib_handle, "get_name") != NULL;
  bool has_options = dlsym(lib_handle, "get_options") != NULL;
  bool has_module = dlsym(lib_handle, "get_module") != NULL;
  bool has_source_id = dlsym(lib_handle, "is_source") != NULL;
  
  
  dlclose(lib_handle);

  std::cout << "guid: " << has_guid << std::endl;
  std::cout << "name: " << has_name << std::endl;
  std::cout << "opts: " << has_options << std::endl;
  std::cout << "has source: " << has_source_id << std::endl;
  std::cout << "desc: " << has_description << std::endl;
  std::cout << "module: " << has_module << std::endl;
  return has_guid && has_name && has_description && has_module && has_options && has_source_id;
}


namespace fn_dag {
  

  module::module() {}

  MODULE_TYPE module::get_type() {
    return MODULE_TYPE::UNDEFINED;
  }

  module_source *module::get_handle_as_source() {
    return nullptr;
  }

  module_transmit *module::get_handle_as_mapping() {
    return nullptr;
  }

  source_handler::source_handler(module_source *_handle) : handler(_handle) {}
  source_handler::~source_handler() {}

  MODULE_TYPE source_handler::get_type() {
    return MODULE_TYPE::SOURCE;
  }

  module_source *source_handler::get_handle_as_source() {
    return handler;
  }

  module_handler::module_handler(module_transmit *_handle) : handler(_handle) {}
  module_handler::~module_handler() {}

  MODULE_TYPE module_handler::get_type() {
    return MODULE_TYPE::FILTER;
  }

  module_transmit *module_handler::get_handle_as_mapping() {
    return handler;
  }
}


string fsys_serialize(const vector<fn_dag::library_spec> * const _tree) {
  Json::Value sources;
  Json::Value nodes;
  Json::Value root;
  for(const auto &item : *_tree) {
    Json::Value spec;
    spec["guid"] = item.lib_guid;
    bool is_src = item.is_source;
    Json::Value options;
    for(auto option : item.instantiation_options) {
      Json::Value option_val;
      option_val["id"] = option.serial_id;
      switch(option.type) {
        case fn_dag::OPTION_TYPE::STRING:
          option_val["val"] = option.value.string_value;
          break;
        case fn_dag::OPTION_TYPE::INT:
          option_val["val"] = option.value.int_value;
          break;
        case fn_dag::OPTION_TYPE::BOOL:
          option_val["val"] = option.value.bool_value;
          break;
      }
      options.append(option_val);
    }
    if(options.size() > 0) 
      spec["opts"] = options;

    if(is_src) {
      sources[item.name] = spec;
    } else {
      spec["parent"] = item.parent_name;
      nodes[item.name] = spec;
    }
  }

  root["sources"] = sources;
  root["nodes"] = nodes;
  Json::FastWriter fastWriter;
  return fastWriter.write(root);;
}

static uint32_t __get_guid(Json::Value guid_node) {
  Json::Value guid = guid_node["guid"];
  if(!guid.isNull() && guid.isUInt())
    return guid.asUInt();
  return 0;
}

static fn_dag::lib_options __generate_options(Json::Value spec_in, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &library) {
  fn_dag::lib_options dest_options;

  Json::Value options = spec_in["opts"];
  
  if(!options.isNull()) {
    for(int i = 0;i < options.size();i++) {
      fn_dag::construction_option dest_option;
      Json::Value serial_id_string = options[i]["id"];
      Json::Value serial_value = options[i]["val"];
      if(!serial_value.isNull() && !serial_id_string.isNull()) {
        dest_option.serial_id = serial_id_string.asUInt();
        
        if(serial_value.isInt()) {
          dest_option.type = fn_dag::OPTION_TYPE::INT;
          dest_option.value.int_value = serial_value.asInt();
        } else if (serial_value.isBool()) {
          dest_option.type = fn_dag::OPTION_TYPE::BOOL;
          dest_option.value.bool_value = serial_value.asBool();
        } else if (serial_value.isString()) {
          dest_option.type = fn_dag::OPTION_TYPE::STRING;
          dest_option.value.string_value = strdup(serial_value.asString().c_str());
        } else
          std::cout << "Warning, unknown converstion for value: " << serial_value << std::endl;
        dest_options.push_back(dest_option);
      }
    }
  }
  return dest_options;
}

static std::shared_ptr<fn_dag::module> __instantiate_from_library(Json::Value node, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &library) {
  std::cout << "JSON: " << node << std::endl;
  long s_guid = __get_guid(node);
  std::cout << " Guid: " << s_guid << std::endl;
  if(s_guid != 0) {
    fn_dag::instantiate_fn spec_creator = nullptr;
    if(library.contains(s_guid))
      spec_creator = library.at(s_guid);
    else
      return nullptr;
    // std::shared_ptr<fn_dag::library_spec> spec;
    // auto lib_it = std::find_if(library.begin(), library.end(), 
    //                         [s_guid](std::shared_ptr<fn_dag::lib_specification> candidate) {
    //                           return candidate->serial_id_guid == s_guid;
    //                         });
    // if(lib_it == library.end())
    //   return nullptr;
    // spec = *lib_it;
    
    fn_dag::lib_options dest_options = __generate_options(node, library);
    return spec_creator(&dest_options);
  }
  return nullptr;
}

fn_dag::dag_manager<std::string> *fsys_deserialize(const std::string &json_in, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &library) {
  fn_dag::dag_manager<std::string> *manager = new fn_dag::dag_manager<std::string>();
  fn_dag::__g_run_single_threaded = true;
  int num_nodes_created = 0;
  Json::Reader string_reader;
  Json::Value root;
  Json::Value sources;
  Json::Value nodes;
  
  string_reader.parse(json_in, root, false);
  sources = root["sources"];
  nodes = root["nodes"];

  for(std::string name : sources.getMemberNames()) {
    std::shared_ptr<fn_dag::module> lib_handle = __instantiate_from_library(sources[name], library);
    if(lib_handle != nullptr)
      manager->add_dag(name, lib_handle->get_handle_as_source(), true);
  }

  for(std::string name : nodes.getMemberNames()) {
    Json::Value parent_value = nodes[name]["parent"];
    
    if(!parent_value.isNull() && parent_value.isString()) {
      std::string parent_name = parent_value.asString();
      std::cout <<" INSTA " << name << " WITH PAR " << parent_name << std::endl;
      if(manager->manager_contains_id(parent_name)) {
        std::cout <<" manager contains id, instantiating\n";
        std::shared_ptr<fn_dag::module> lib_handle = __instantiate_from_library(nodes[name], library);
        std::cout << "got lib handle\n";
        if(lib_handle != nullptr) {
          std::cout << "lib handle not null\n";
          manager->add_node(name, lib_handle->get_handle_as_mapping(), parent_name);
        }
          
      } else {
        std::cerr << "no parent available for id " << name << std::endl;
      }
    }
  }

  return manager;
}