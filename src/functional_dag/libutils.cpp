// ---------------------------------------------
//    ___                 .___
//   |_  \              __| _/____     ____
//    /   \    ______  / __ |\__  \   / ___\
//   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
//  /_/  \__\         \____ |(____  /\___  /
//                         \/     \//_____/
// ---------------------------------------------
// @author ndepalma@alum.mit.edu
#include <dlfcn.h>
#include <functional_dag/dlpack.h>
#include <functional_dag/lib_utils.h>
#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional_dag/dag_interface.hpp>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

#include "fb_gen/lib_spec_generated.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "functional_dag/incbin_util.h"

#ifdef __APPLE__
static const string dylib_suffix("dylib");
#elif __linux__
static const string dylib_suffix("so");
#endif

using namespace std;

INCBIN(g, schema, STR(SCHEMA_FILE));

static flatbuffers::Parser g_parser;
static bool has_initialized = false;

flatbuffers::Parser *__get_parser() {
  if (has_initialized) return &g_parser;

  std::string pipe_spec_data{
      g_schema_start,
      static_cast<size_t>((char *)&g_schema_end - (char *)&g_schema_start)};

  if (!g_parser.Deserialize(reinterpret_cast<uint8_t *>(pipe_spec_data.data()),
                            pipe_spec_data.size())) {
    std::cerr << "Issue deserializing schema!" << std::endl;
    return nullptr;
  }

  has_initialized = true;
  return &g_parser;
}

[[nodiscard]] vector<fs::directory_entry> get_all_available_libs(
    const fs::directory_entry &library_path) {
  vector<fs::directory_entry> all_files{};

  if (library_path.exists()) {
    for (const auto &entry : fs::directory_iterator(library_path.path()))
      if (entry.path().string().ends_with(dylib_suffix))
        all_files.push_back(entry);
  } else
    cerr << "Could not find library directory: " << library_path.path().string()
         << endl;
  return all_files;
}

bool library::preflight_lib(const fs::path _lib_path) {
#ifdef __APPLE__
  void *const lib_handle =
      dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_FIRST);
#else
  void *const lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif

  bool has_node_details = dlsym(lib_handle, "get_library_details") != nullptr;
  bool has_constructor = dlsym(lib_handle, "construct_node") != nullptr;
  dlclose(lib_handle);

  return has_node_details && has_constructor;
}

void library::load_lib(const fs::path _lib_path) {
  // Open the dynamic library
  void *const lib_handle =
      dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_FIRST);

  // Resolve the function to get details about the dynamic library
  void *get_lib_details_fn = dlsym(lib_handle, "get_library_details");
  std::function<get_library_fn_type> get_library_details(
      reinterpret_cast<get_library_fn_type *>(get_lib_details_fn));

  // Finally get the nodes that can be constructed from this library
  library_spec specification = get_library_details();
  for (auto &spec : specification.available_nodes) {
    GUID<node_spec> guid{spec.guid._id};
    construction_signature *constructor =
        reinterpret_cast<construction_signature *>(
            dlsym(lib_handle, "construct_node"));

    constructors[guid] = std::function<construction_signature>(constructor);

    switch (spec.module_type) {
      case fn_dag::NODE_TYPE_SOURCE:
        break;
      case fn_dag::NODE_TYPE_FILTER:
        break;
      default:
        std::cerr << "Module type unsupported:" << spec.module_type
                  << std::endl;
    }
  }
}

string fsys_serialize(const uint8_t *serialized_dag) {
  const flatbuffers::Parser *parser = __get_parser();
  std::string jsongen;
  GenerateText(*parser, serialized_dag, &jsongen);
  return jsongen;
}

void library::_create_node(dag_manager<std::string> &manager,
                           const fn_dag::node_spec *spec) {
  const fn_dag::GUID_vals *s_guid = spec->target_id();
  if (s_guid != nullptr) {
    const fn_dag::GUID<node_spec> guid(*s_guid);
    if (constructors.contains(guid)) {
      std::function<construction_signature> spec_creator =
          constructors.at(guid);
      if (!spec_creator(manager, *spec)) {
        std::cerr << "Unable to construct node: " << spec->name()->c_str()
                  << std::endl;
      }
    }
  }
}

fn_dag::dag_manager<std::string> *library::fsys_deserialize(
    const std::string &json_in) {
  flatbuffers::Parser *const parser = __get_parser();
  if (!parser->ParseJson(json_in.c_str())) {
    std::cerr << "Error parsing JSON: " << parser->error_.c_str() << std::endl;
    return nullptr;
  }

  std::set<std::string_view> nodes_added;
  fn_dag::dag_manager<std::string> *manager =
      new fn_dag::dag_manager<std::string>();
  const flatbuffers::FlatBufferBuilder &buffer = parser->builder_;

  flatbuffers::Verifier verifier(buffer.GetBufferPointer(), buffer.GetSize());
  if (fn_dag::Verifypipe_specBuffer(verifier)) {
    const auto *pipe_spec = fn_dag::Getpipe_spec(buffer.GetBufferPointer());

    ////////////////////////////////////////////////
    /// Begin by instantiating all of the nodes
    const auto *vec = pipe_spec->sources();

    for (uint32_t i = 0; i < vec->size(); i++) {
      const auto *nodes_spec = vec->Get(i);
      _create_node(*manager, nodes_spec);
      nodes_added.emplace(nodes_spec->name()->string_view());
    }

    ////////////////////////////////////////////////
    /// Figure out the order to create the nodes of the tree.
    std::vector<int> ordered_list;
    std::list<int> to_sort(pipe_spec->nodes()->size());
    std::set<std::string> node_names;
    std::iota(to_sort.begin(), to_sort.end(), 0);

    bool has_found_node = true;
    while (!to_sort.empty() && has_found_node) {
      has_found_node = false;
      std::list<int> to_remove;
      for (uint32_t i : to_sort) {
        const fn_dag::node_spec *nodes_spec = pipe_spec->nodes()->Get(i);

        // Check if contains all
        if (std::all_of(
                nodes_spec->wires()->cbegin(), nodes_spec->wires()->cend(),
                [&manager,
                 &node_names](const fn_dag::string_mapping *x) -> bool {
                  return manager->manager_contains_id(x->value()->str()) ||
                         node_names.contains(x->value()->str());
                })) {
          to_remove.push_back(i);
          ordered_list.push_back(i);
          has_found_node = true;
          node_names.emplace(nodes_spec->name()->str());
        }
      }
      std::for_each(to_remove.begin(), to_remove.end(),
                    [&to_sort](int i) { to_sort.remove(i); });
    }

    ////////////////////////////////////////////////
    /// Finally create the nodes of the tree
    std::for_each(ordered_list.cbegin(), ordered_list.cend(),
                  [&manager, &pipe_spec, this](const int i) {
                    const fn_dag::node_spec *nodes_spec =
                        pipe_spec->nodes()->Get(i);
                    _create_node(*manager, nodes_spec);
                  });
  }

  return manager;
}
