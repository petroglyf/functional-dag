/**
 *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * If a user decides to implement the functionals themselves, then they can use these interfaces.
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <filter_sys/dag_interface.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <torch/torch.h>
#pragma GCC diagnostic pop

#include <filter_sys/filter_sys.hpp>

using namespace std;
namespace fs = std::filesystem;

namespace fn_dag {
  typedef enum {
    SOURCE, FILTER, COMBINER, SINK, UNDEFINED
  } MODULE_TYPE;

  typedef fn_dag::dag_source<torch::Tensor> module_source;
  typedef fn_dag::dag_node<torch::Tensor, torch::Tensor> module_transmit;

  class module {
  public:
    module();
    virtual ~module() = default;

    virtual MODULE_TYPE get_type();
    virtual module_source *get_handle_as_source();
    virtual module_transmit *get_handle_as_mapping();
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
    module_transmit *get_handle_as_mapping();
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