#pragma once
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

#include <functional_dag/dlpack.h>

#include <filesystem>
#include <functional_dag/dag_interface.hpp>
#include <functional_dag/filter_sys.hpp>
#include <functional_dag/guid_impl.hpp>
#include <map>
#include <string>
#include <vector>

#include "fb_gen/lib_spec_generated.h"

namespace fn_dag {
using namespace std;
namespace fs = std::filesystem;

using construction_signature = bool(dag_manager<string> &, const node_spec &);

/// This defines the options to a node, e.g. how to configure it
typedef struct option_spec {
  OPTION_TYPE type = OPTION_TYPE_UNDEFINED;
  string name;
  string option_prompt;
  string short_description;
} option_spec;

/// A node spec defines the basic properties of a node. A library is an array of
/// nodes.
typedef struct node_prop_spec node_prop_spec;

typedef struct node_prop_spec {
  GUID<node_prop_spec> guid;
  NODE_TYPE module_type = NODE_TYPE::NODE_TYPE_UNDEFINED;
  vector<option_spec> construction_types;
} node_prop_spec;

/// Library definition
class library {
 protected:
  map<GUID<node_spec>, function<construction_signature>> constructors;
  map<GUID<node_spec>, vector<option_spec>> options;

  expected<bool, fn_dag::error_codes> _create_node(
      dag_manager<string> &manager, const fn_dag::node_spec *const spec);

 public:
  library() = default;
  [[nodiscard]] static bool preflight_lib(const fs::path _lib_path);
  [[nodiscard]] expected<bool, fn_dag::error_codes> load_lib(
      const fs::path _lib_path);
  void load_all_available_libs(const fs::directory_entry &library_path);

  [[nodiscard]] expected<fn_dag::dag_manager<string> *, fn_dag::error_codes>
  fsys_deserialize(const string &json_in);
};

[[nodiscard]] expected<vector<fs::directory_entry>, error_codes>
get_all_available_libs(const fs::directory_entry &library_path);

expected<string, error_codes> fsys_serialize(const uint8_t *const);

typedef struct library_spec {
  GUID<library> guid;
  vector<node_prop_spec> available_nodes;
} library_spec;

using get_library_fn_type = library_spec();
}  // namespace fn_dag