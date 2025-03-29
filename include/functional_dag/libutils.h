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

#include <filesystem>
#include <functional_dag/dag_interface.hpp>
#include <functional_dag/filter_sys.hpp>
#include <functional_dag/guid_impl.hpp>
#include <map>
#include <string>
#include <vector>

#include "functional_dag/error_codes.h"
#include "functional_dag/lib_spec_generated.h"

namespace fn_dag {
using namespace std;
namespace fs = std::filesystem;

using construction_signature = bool(dag_manager<string> &, const node_spec &);

/// (Library related) This defines the options to a node, eg: how to configure
/// it.
typedef struct option_spec {
  /// The type of the option - i.e. integer, string, etc
  OPTION_TYPE type = OPTION_TYPE_UNDEFINED;
  /// The parameters variable name
  string name;
  /// A prompt to be used if the user is asked to specify the param.
  string option_prompt;
  /// A short description of what the parameter is.
  string short_description;
} option_spec;

/// (Library related) A node spec defines the basic properties of a node.
typedef struct node_prop_spec {
  /// The node's unique identifier.
  GUID<node_prop_spec> guid;
  /// The name of the generic node.
  string name;
  /// Description of the generic node.
  string description;
  /// The type of module - whether it generates data from a sensor or processes
  /// data.
  NODE_TYPE module_type = NODE_TYPE::NODE_TYPE_UNDEFINED;
  /// The options that should be specified to create the node.
  vector<option_spec> construction_types;
} node_prop_spec;

struct library_spec;  // a forward declaration of the library_spec struct

/** (Library related) A library defines what is needed to construct a dag tree.
 *
 * The library is a utility that defines an interface for dynamic libraries to
 * be loaded into the library. Once a set number of nodes are available to be
 * constructed from the dynamic libraries, users can define a "pipe" which
 * represents the dag to be added to a manager to be managed.
 */
class library {
 protected:
  /// A mapping between IDs to the constructors that create the nodes; The user
  /// will need to define these libraries.constructors in their dynamic
  /// libraries.
  map<GUID<node_spec>, function<construction_signature>> m_constructors;

  /// This is a parallel data structure that ensures that whatever
  /// options/parameters are passed to it at runtime conform to the expectations
  /// of the constructors signature.
  map<GUID<node_spec>, vector<option_spec>> m_options;

  /** Private function to verify parameters, create the node, and attach it.
   *
   * @param _manager A dag manager to attach the new node to.
   * @param _spec Specifications about which node and what parameters should be
   * used to create it.
   * @return If successful, returns true but if not, then it returns an error
   * code.
   */
  expected<bool, fn_dag::error_codes> _create_node(
      dag_manager<string> &_manager, const fn_dag::node_spec *const _spec);

  /// This is a list of all of the libraries that have been loaded so far.
  std::vector<library_spec> m_library_specs;

 public:
  library() = default;

  /** A way to iterate through the loaded libraries.
   *
   * @return A read only iterator view for the loaded library specs.
   */
  [[nodiscard]] std::span<const library_spec> get_spec_iter();

  /** A quick and easy function to test whether a dynamic library conforms to
   * the library specifications.
   *
   * If the user of the library would like to check if a dynamic library could
   * be tracked and used to create nodes, then this function will help you
   * check. It's meant to be a stateless way to just double check before asking
   * the library to load it.
   *
   * @param _lib_path The path to the dynamic library
   * @return True if loadable; false otherwise.
   */
  [[nodiscard]] static bool preflight_lib(const fs::path _lib_path);

  /** This function loads the library and keeps the constructor around for
   * subsequent asks to create the node.
   *
   * It is recommended to preflight the library before loading it but not
   * necessary. This is only due to the heavyweight nature of this funciton
   * compared to preflight. Once The library is known to be compatible with the
   * signatures, then this function will load the dynamic library and store it's
   * constructors and specifications to be used later during construction.
   *
   * @param _lib_path The path to the dynamic library.
   * @return True if successfully loaded; otherwise an error code is passed
   * providing more info.
   */
  [[nodiscard]] expected<bool, fn_dag::error_codes> load_lib(
      const fs::path _lib_path);

  /** This function is for convenience.
   *
   * This function takes a directory and iterates through the .so or .dylibs. It
   * will preflight them and load them if they are indeed compatible.
   *
   * @param _library_path A path or list of paths where many .so and .dylibs are
   * expected to be.
   * @param _logger An output stream for potential logging. Defaults to stdout.
   */
  expected<bool, error_codes> load_all_available_libs(
      const fs::directory_entry &_library_path, ostream &_logger = cout);
  void load_all_available_libs(
      const vector<fs::directory_entry> &_library_paths,
      ostream &_logger = cout);

  /** Takes highly structured JSON in and turns it into a lambda dag.
   *
   * In some ways this can be considered the casual user's interface to this
   * entire library. This function will connect the functional dag's
   * functionality to a simple JSON specification. It will check if the node
   * specified in the JSON file is available.
   *
   * @param _json_in The JSON specification to create the DAG structure.
   * @param run_single_threaded Whether or not to run the DAGs on the same
   * thread. This is useful for debugging but not recommended for production
   * code. (optional)
   * @return A normal dag manager to start/stop/modify if successful and an
   * error if unsuccessful.
   */
  [[nodiscard]] expected<fn_dag::dag_manager<string> *, fn_dag::error_codes>
  fsys_deserialize(const string &_json_in,
                   const bool run_single_threaded = false);
};

/** Similar to load_all_available_libs, this function will retreive compatible
 * libraries in the directory.
 *
 * This function iterates through a directory and returns the compatible
 * dynamicly loadable library.
 *
 * @param _library_path A path where many .so and .dylibs are expected to be.
 * @return A list of compatible files or an error code if unsucessful.
 */
[[nodiscard]] expected<vector<fs::directory_entry>, error_codes>
get_all_available_libs(const fs::directory_entry &_library_path);

/** (Beta) This function will take a spec in and serialize it back out to JSON.
 *
 * Since the DAG does not contain all of the information available for
 * serialization, this function will take a pipe_spec as a flatbuffer in byte
 * buffer format. It will check the basic structure and then serialize it into
 * JSON. Since the schema is needed for serialization, it makes sense to have it
 * here. The beta functionality here is only due to the inconvienience of
 * dealing with pipe specs without helper functions. For an example, check out
 * the lib_test.cpp unit test example of constructing a pipe_spec.
 *
 * @param _buffer_in The byte buffer in to be serialized to JSON.
 * @return The contents of a JSON file to be written out if necessary.
 */
expected<string, error_codes> fsys_serialize(const uint8_t *const _buffer_in);

/// (Library related) A library spec defines the interface a dynamicly loaded
/// library must define to participate.
typedef struct library_spec {
  /// The unique identifier of the library.
  GUID<library> guid;
  /// Which nodes are available in this library.
  vector<node_prop_spec> available_nodes;
} library_spec;

using get_library_fn_type = library_spec();
}  // namespace fn_dag