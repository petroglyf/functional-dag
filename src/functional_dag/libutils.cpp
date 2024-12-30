/** ---------------------------------------------
 *    ___                 .___
 *   |_  \              __| _/____     ____
 *    /   \    ______  / __ |\__  \   / ___\
 *   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 *  /_/  \__\         \____ |(____  /\___  /
 *                         \/     \//_____/
 * ---------------------------------------------
 * @author ndepalma@alum.mit.edu
 */
#include <dlfcn.h>
#include <functional_dag/lib_utils.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional_dag/dag_interface.hpp>
#include <list>
#include <numeric>
#include <set>
#include <string>
#include <vector>

#include "fb_gen/lib_spec_generated.h"
#include "flatbuffers/flatbuffer_builder.h"
#include "flatbuffers/idl.h"
#include "functional_dag/error_codes.h"
#include "functional_dag/incbin_util.h"

INCBIN(g, schema, STR(SCHEMA_FILE));

static flatbuffers::Parser
    g_parser;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static bool has_initialized = false;

auto __get_parser()
    -> std::expected<flatbuffers::Parser *, fn_dag::error_codes> {
  if (has_initialized) return &g_parser;

  std::string pipe_spec_data(
      g_schema_start,
      static_cast<size_t>((char *)&g_schema_end - (char *)&g_schema_start));

  if (!g_parser.Deserialize(reinterpret_cast<uint8_t *>(pipe_spec_data.data()),
                            pipe_spec_data.size())) {
    return std::unexpected(fn_dag::error_codes::SCHEMA_READ_ERROR);
  }

  has_initialized = true;
  return &g_parser;
}

namespace fn_dag {
#ifdef __APPLE__
static const string dylib_suffix("dylib");
#elif __linux__
static const string dylib_suffix("so");
#endif

[[nodiscard]] auto get_all_available_libs(
    const fs::directory_entry &_library_path)
    -> expected<vector<fs::directory_entry>, error_codes> {
  vector<fs::directory_entry> all_files(0);

  if (_library_path.exists()) {
    for (const auto &entry : fs::directory_iterator(_library_path.path()))
      if (entry.path().string().ends_with(dylib_suffix))
        all_files.push_back(entry);
  } else {
    return unexpected(error_codes::PATH_DOES_NOT_EXIST);
  }

  return all_files;
}

[[nodiscard]] auto library::preflight_lib(const fs::path _lib_path) -> bool {
  void *lib_handle = nullptr;
  bool has_node_details = false;
  bool has_constructor = false;
#ifdef __APPLE__
  lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_FIRST);
#else
  lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif

  has_node_details = dlsym(lib_handle, "get_library_details") != nullptr;
  has_constructor = dlsym(lib_handle, "construct_node") != nullptr;
  dlclose(lib_handle);

  return has_node_details && has_constructor;
}

auto library::load_lib(const fs::path _lib_path)
    -> expected<bool, error_codes> {
  // Open the dynamic library
  void *lib_handle = nullptr;
  function<get_library_fn_type> get_library_details(nullptr);
#ifdef __APPLE__
  lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_FIRST);
#else
  lib_handle = dlopen(_lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif

  if (lib_handle == nullptr) {
    return unexpected(error_codes::PATH_DOES_NOT_EXIST);
  }

  // Resolve the function to get details about the dynamic library
  void *get_lib_details_fn = dlsym(lib_handle, "get_library_details");
  if (get_lib_details_fn == nullptr) {
    return unexpected(error_codes::NO_DETAILS);
  }

  get_library_details =
      reinterpret_cast<get_library_fn_type *>(get_lib_details_fn);

  // Finally get the nodes that can be constructed from this library
  const library_spec specification = get_library_details();
  for (const auto &spec : specification.available_nodes) {
    const GUID<node_spec> guid{spec.guid.m_id};
    construction_signature *constructor =
        reinterpret_cast<construction_signature *>(
            dlsym(lib_handle, "construct_node"));
    if (constructor == nullptr) {
      return unexpected(error_codes::NO_CONSTRUCTOR);
    }

    m_constructors[guid] = function<construction_signature>(constructor);

    switch (spec.module_type) {
      case fn_dag::NODE_TYPE_SOURCE:
        break;
      case fn_dag::NODE_TYPE_FILTER:
        break;
      default:
        return unexpected(error_codes::MODULE_TYPE_UNSUPPORTED);
    }
  }

  return true;
}

auto fsys_serialize(const uint8_t *const _buffer_in)
    -> expected<string, error_codes> {
  string json_storage;  // NOLINT(cppcoreguidelines-init-variables)
  const auto parser = __get_parser();
  if (parser.has_value()) {
    auto error_str = GenerateText(*parser.value(), _buffer_in, &json_storage);
    if (error_str) {
      return unexpected(error_codes::SERIALIZATION_ERROR);
    }
    return json_storage;
  }
  return unexpected(parser.error());
}

auto library::_create_node(dag_manager<string> &_manager,
                           const node_spec *_spec)
    -> expected<bool, error_codes> {
  const GUID_vals *s_guid = _spec->target_id();
  if (s_guid != nullptr) {
    const GUID<node_spec> guid(*s_guid);
    if (m_constructors.contains(guid)) {
      auto spec_creator = m_constructors.at(guid);
      if (!spec_creator(_manager, *_spec)) {
        return unexpected(error_codes::CONSTRUCTION_FAILED);
      }
      return true;
    }
    return unexpected(error_codes::DAG_NOT_FOUND);
  }
  return unexpected(error_codes::GUID_CONSTRUCTION_FAILED);
}

[[nodiscard]] auto library::fsys_deserialize(const string &_json_in)
    -> expected<dag_manager<string> *, error_codes> {
  const auto parser = __get_parser();
  if (parser.has_value()) {
    if (!parser.value()->ParseJson(_json_in.c_str())) {
      return unexpected(error_codes::JSON_PARSER_ERROR);
    }

    set<string_view> nodes_added({});
    auto manager = new dag_manager<string>();
    const flatbuffers::FlatBufferBuilder &buffer = parser.value()->builder_;

    flatbuffers::Verifier verifier(buffer.GetBufferPointer(), buffer.GetSize());
    if (Verifypipe_specBuffer(verifier)) {
      const auto *pipe_spec = Getpipe_spec(buffer.GetBufferPointer());

      ////////////////////////////////////////////////
      /// Begin by instantiating all of the nodes
      const auto vec = pipe_spec->sources();

      for (uint32_t i = 0; i < vec->size(); i++) {
        const auto *nodes_spec = vec->Get(i);
        if (const auto parent = _create_node(*manager, nodes_spec); parent) {
          nodes_added.emplace(nodes_spec->name()->string_view());
        } else {
          return unexpected(parent.error());
        }
      }

      ////////////////////////////////////////////////
      /// Figure out the order to create the nodes of the tree.
      vector<int> ordered_list({});
      list<int> to_sort(pipe_spec->nodes()->size());
      set<string> node_names({});
      iota(to_sort.begin(), to_sort.end(), 0);

      bool has_found_node = true;
      while (!to_sort.empty() && has_found_node) {
        has_found_node = false;
        list<int> to_remove({});
        for (uint32_t i : to_sort) {
          const node_spec *nodes_spec = pipe_spec->nodes()->Get(i);

          // Check if contains all
          if (all_of(nodes_spec->wires()->cbegin(), nodes_spec->wires()->cend(),
                     [&manager, &node_names](const string_mapping *x) -> bool {
                       return manager->manager_contains_id(x->value()->str()) ||
                              node_names.contains(x->value()->str());
                     })) {
            to_remove.push_back(i);
            ordered_list.push_back(i);
            has_found_node = true;
            node_names.emplace(nodes_spec->name()->str());
          }
        }
        for_each(to_remove.begin(), to_remove.end(),
                 [&to_sort](int i) { to_sort.remove(i); });
      }

      if (to_sort.size() > 0) {
        // Something was left over that couldn't be constructed
        return unexpected(error_codes::CONSTRUCTION_FAILED);
      }
      ////////////////////////////////////////////////
      /// Finally create the nodes of the tree
      error_codes some_val = error_codes::NO_DETAILS;
      for_each(ordered_list.cbegin(), ordered_list.cend(),
               [&manager, &pipe_spec, &some_val, this](const int i) {
                 const node_spec *nodes_spec = pipe_spec->nodes()->Get(i);
                 if (auto err = _create_node(*manager, nodes_spec); !err) {
                   some_val = err.error();
                 }
               });
      if (some_val != error_codes::NO_DETAILS) {
        return unexpected(some_val);
      }
    } else {
      return unexpected(error_codes::PIPE_SPEC_ERROR);
    }

    return manager;
  }
  return unexpected(parser.error());
}
}  // namespace fn_dag