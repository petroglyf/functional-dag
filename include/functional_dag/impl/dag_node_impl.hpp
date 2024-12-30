#pragma once
/**
 * ---------------------------------------------
 *    ___                 .___
 *   |_  \              __| _/____     ____
 *    /   \    ______  / __ |\__  \   / ___\
 *   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 *  /_/  \__\         \____ |(____  /\___  /
 *                         \/     \//_____/
 * ---------------------------------------------
 * @author ndepalma@alum.mit.edu
 */

#include <iostream>
#include <string>

#include "functional_dag/core/dag_utils.hpp"
#include "functional_dag/dag_interface.hpp"
#include "functional_dag/impl/dag_fanout_impl.hpp"

namespace fn_dag {
using namespace std;

// Forward declaration of dag_fanout_node
template <typename Out, typename IDType>
class dag_fanout_node;

/** An internal pure virtual interface for internal nodes
 *
 * This should not be used by users. It is simply a pure virtual class for
 * internal nodes
 */
template <class Type, class IDType>
class _abstract_internal_dag_node {
 public:
  /** Pure, default deconstructor */
  virtual ~_abstract_internal_dag_node() = default;
  /** Must provide a way to get an ID. Could be string or int or something
   * efficient. */
  virtual const IDType &get_id() = 0;
  /** Must provide a way to call the function */
  virtual void run_filter(const Type *const _data) = 0;
  /** Must provide a way to print some diagnostics to screen. */
  virtual void print(const string &_plus) = 0;
};

/** An internal class to encapsulate a function that transmutes input data to
 * output data. */
template <typename In, typename Out, typename IDType>
class _internal_dag_node : public _abstract_internal_dag_node<In, IDType> {
  friend class dag_fanout_node<In, IDType>;  // Friend classing fanout for the
                                             // add_node search

 private:
  dag_node<In, Out> *m_node_hook;  // The function to run
  const IDType m_node_id;          // The ID of the node
  dag_fanout_node<Out, IDType>
      *m_child;  // All of the children to provide our output data to.
  const fn_dag::_dag_context
      &g_context;  // A hook to the global context of this DAG.

 public:
  /** Internal constructor for the encapsulated lambda function.
   *
   * This constructor simply initializes the internal variables that this
   * class is meant to encapsulate.
   *
   * @param _node_id The new ID of the node.
   * @param _node The function to call when data comes in.
   * @param _context The state variables for the DAG. All nodes share this
   * information.
   */
  _internal_dag_node(IDType _node_id, dag_node<In, Out> *_node,
                     const fn_dag::_dag_context &_context)
      : m_node_hook(_node),
        m_node_id(_node_id),
        m_child(new dag_fanout_node<Out, IDType>(_context)),
        g_context(_context) {}

  /** Default constructor */
  ~_internal_dag_node() {
    delete m_child;
    delete m_node_hook;
  }

  /** Runs the lambda function and passes it to this nodes children.
   *
   * This function simply encapsulates the process of calling update on the
   * input data, collecting the output data and propagating it to all of the
   * children to be processed.
   *
   * @param _data Input data to process by the node.
   */
  void run_filter(const In *const _data) {
    unique_ptr<Out> data_out = m_node_hook->update(_data);
    if (!g_context.filter_off && data_out != nullptr)
      m_child->fan_out(std::move(data_out));
  }

  /** Print function
   *
   * Simply prints the node's ID and asks the children to do the same.
   *
   * @param _indent_str How to indent the children
   */
  void print(const string &_indent_str) {
    *g_context.log << m_node_id << endl;
    m_child->print(_indent_str);
  }

  /** Getter for the ID
   *
   * Simply, a getter for the internal ID of the node this class encapsulates.
   *
   * @return ID of the node
   */
  const IDType &get_id() { return m_node_id; }
};
}  // namespace fn_dag
