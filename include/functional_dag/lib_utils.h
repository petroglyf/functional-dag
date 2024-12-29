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

using namespace std;
using namespace fn_dag;
namespace fs = std::filesystem;

using construction_signature = bool(dag_manager<std::string> &,
                                    const node_spec &);

/// This defines the options to a node, e.g. how to configure it
typedef struct option_spec {
  OPTION_TYPE type = OPTION_TYPE_UNDEFINED;
  std::string name;
  string option_prompt;
  string short_description;
} option_spec;

/// A node spec defines the basic properties of a node. A library is an array of
/// nodes.
typedef struct node_prop_spec node_prop_spec;

typedef struct node_prop_spec {
  GUID<node_prop_spec> guid;
  NODE_TYPE module_type = NODE_TYPE::NODE_TYPE_UNDEFINED;
  std::vector<option_spec> construction_types;
} node_prop_spec;

/// Library definition
class library {
 protected:
  std::map<GUID<node_spec>, std::function<construction_signature>> constructors;
  std::map<GUID<node_spec>, std::vector<option_spec>> options;

  void _create_node(dag_manager<std::string> &manager,
                    const fn_dag::node_spec *spec);

 public:
  library() = default;
  static bool preflight_lib(const fs::path _lib_path);
  void load_lib(const fs::path _lib_path);
  void load_all_available_libs(const fs::directory_entry &library_path);

  [[nodiscard]] dag_manager<std::string> *fsys_deserialize(
      const std::string &json_in);

  static vector<fs::directory_entry> get_all_available_libs(
      const fs::directory_entry &library_path);
};

string fsys_serialize(const uint8_t *serialized_dag);

typedef struct library_spec {
  GUID<library> guid;
  std::vector<node_prop_spec> available_nodes;
} library_spec;

using get_library_fn_type = library_spec();
