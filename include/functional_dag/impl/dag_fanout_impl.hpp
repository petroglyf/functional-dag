#pragma once
/**
 * ---------------------------------------------
 *    ___                 .___
 *   |_  \              __| _/____     ______
 *    /   \    ______  / __ |\__  \   / ___  /
 *   / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 *  /_/  \__\         \____ |(____  /\___  /
 *                         \/     \//_____/
 * ---------------------------------------------
 * @author ndepalma@alum.mit.edu
 */

#include <functional_dag/core/dag_utils.hpp>
#include <functional_dag/impl/dag_node_impl.hpp>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace fn_dag {

/** Internal node to take data from parent and run children.
 *
 * Every node when generating output will send it's output to the nodes
 * children. All child nodes run with the output data being immutable.
 */
template <typename Type, typename IDType>
class dag_fanout_node {
 private:
  const fn_dag::_dag_context &g_context;  // Shared state
  vector<_abstract_internal_dag_node<Type, IDType> *>
      m_children;  // Children to fan-out to

  /**
   * This is an internal function for adding subsequent nodes
   * @param _new_node The node to add to the children
   */
  void add_node(_abstract_internal_dag_node<Type, IDType> *_new_node) {
    m_children.push_back(_new_node);
  }

 public:
  /** This node uses data computed from the previous node to fan-out to it's
   children
   *
   * This node specifically fans data out and makes sure the children can't
   modify the
   * data as well as cleaning up the data on the heap when finished

   * @param _context The shared state between the nodes
  */
  dag_fanout_node(const _dag_context &_context)
      : g_context(_context), m_children() {}
  /** Standard deconstructor */
  ~dag_fanout_node() {
    for (auto internal_dag : m_children) delete internal_dag;
    m_children.clear();
  }

  /** Function to do it's job
   *
   * This function will take data from the parent node and execute
   * the subsequent functions with the data. After the subsequent
   * nodes have been run, it deletes the data. The nodes will run
   * on their own threads in the event that run_single_threaded is
   * off. Otherwise, this function will block until the children are
   * finished in a depth-first way.
   *
   * @param _data Data from the parent node
   */
  void fan_out(Type *_data) {
    if (!g_context.run_single_threaded) {
      std::vector<thread> child_threads;

      for (auto it : m_children)
        child_threads.push_back(thread(
            &fn_dag::_abstract_internal_dag_node<Type, IDType>::run_filter, it,
            std::ref(_data)));

      for (uint32_t i = 0; i < child_threads.size(); i++)
        child_threads[i].join();
    } else
      for (auto it : m_children) it->run_filter(_data);

    delete _data;
  }

  /** Printing function
   *
   * This will print recursively to the logging stream the identity
   * of the nodes. This can help ensure the dag was constructed correctly
   *
   * @param _indent The parents indent context
   */
  void print(string _indent) {
    string plus = _indent + g_context.indent_str;
    for (auto child : m_children) {
      *g_context.log << _indent << "->";
      child->print(plus);
    }
  }

  /** Recursively adds a node to children.
   *
   * This function will check whether the node attaches to the parent
   * of this fan-out. If so, then it becomes a child of the parent function.
   * If it did not attach to parent node, then it will recursively check its
   * children until it finds it. If the parent isn't found, it will return
   * false.
   *
   * @param _node_to_add The node to add
   * @param _onto The ID of the parent the node wants to attach to
   * @param _parent_id The current ID of this nodes parent
   * @return True if it was attached to the graph. Otherwise false.
   */
  template <typename In, typename Out>
  bool add_node_to_subdag(_internal_dag_node<In, Out, IDType> *_node_to_add,
                          const IDType _onto, const IDType _parent_id) {
    if (_onto == _parent_id) {
      add_node((_abstract_internal_dag_node<In, IDType> *)_node_to_add);
      return true;
    } else {
      for (auto child = m_children.cbegin(); child != m_children.cend();
           child++) {
        auto *internal_child =
            static_cast<_internal_dag_node<In, Type, IDType> *>(*child);
        fn_dag::dag_fanout_node<Type, IDType> *fanout_node =
            internal_child->m_child;
        if (fanout_node->add_node_to_subdag(_node_to_add, _onto,
                                            (*child)->get_id()))
          return true;
      }
    }

    return false;
  }
};
}  // namespace fn_dag
