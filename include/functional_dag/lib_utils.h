#pragma once
// ---------------------------------------------
//    ___                 .___  
//   |_  \              __| _/____     ____
//    /   \    ______  / __ |\__  \   / ___\
//   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
//  /_/  \__\         \____ |(____  /\___  /
//                         \/     \//_____/
// ---------------------------------------------
// @author ndepalma@alum.mit.edu

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <functional_dag/dag_interface.hpp>
#include <functional_dag/filter_sys.hpp>
#include <functional_dag/dlpack.h>

using namespace std;
namespace fs = std::filesystem;

namespace fn_dag {
  typedef enum {
    SOURCE, FILTER, COMBINER, SINK, UNDEFINED
  } MODULE_TYPE;

  typedef fn_dag::dag_source<DLTensor> module_source;
  typedef fn_dag::dag_node<DLTensor, DLTensor> module_transmit;

  class module {
  public:
    module();
    virtual ~module() = default;

    virtual MODULE_TYPE get_type();
    virtual std::vector<std::string> const get_available_slots();
    virtual module_source *get_handle_as_source();
    virtual module_transmit *get_slot_handle_as_mapping(const std::string &_slot_name);
  };

  class source_handler : public module {
  public: 
    source_handler(module_source *_handle);
    ~source_handler();

    MODULE_TYPE get_type();
    module_source *get_handle_as_source();
  private:
    module_source *handler;
  };

  class module_handler : public module {
  public: 
    module_handler(module_transmit *_handle);
    ~module_handler();

    MODULE_TYPE get_type();
    module_transmit *get_slot_handle_as_mapping(const std::string &_slot_name);
  private:
    module_transmit *handler;
  };

  typedef enum {
    STRING, INT, BOOL
  } OPTION_TYPE;

  typedef struct {
    OPTION_TYPE type;
    union {
      const char * string_value;
      int int_value;
      bool bool_value;
    } value;
    uint32_t serial_id;
    string option_prompt; 
    string short_description;
  } construction_option; 

  typedef vector<construction_option> lib_options;
  // using module_getter_fn = module* (*)(const lib_options *);

  typedef struct {
    uint32_t lib_guid;
    string name;
    string parent_name;
    bool is_source;
    lib_options instantiation_options;
  } library_spec;

  using instantiate_fn = std::function<shared_ptr<module>(const lib_options * const)>;
  // using instantiate_fn = module* (*)(const lib_options * const);
}

std::string fsys_serialize(const vector<fn_dag::library_spec> * const);
fn_dag::dag_manager<std::string> *fsys_deserialize(const std::string &json_in, const std::unordered_map<uint32_t, fn_dag::instantiate_fn> &library);

bool preflight_lib(const fs::path _lib_path);
shared_ptr< vector<fs::directory_entry> > get_all_available_libs(const fs::directory_entry &library_path);