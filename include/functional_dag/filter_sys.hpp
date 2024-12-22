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

#include <functional_dag/dag_interface.hpp>
#include <functional_dag/impl/dag_impl.hpp>

namespace fn_dag {

/** The DAG manager manages all of the dags created by the user. Think of it
 * like a forest of trees.
 *
 * The dag manager is the main interfaces that users should interact with. You
 * should be able to create new root nodes (sources) and add leaves along the
 * way (nodes).
 */
template <typename IDType>
class dag_manager {
 private:
  _dag_context m_context;  // This is the "global" context used by all dags

 public:
  /** All of the DAGs the manager maintains */
  vector<_dag_base<IDType> *> m_all_dags;

  /** Default constructor. Begins in the "on" state and in multi-threaded mode.
   */
  dag_manager() {
    m_context.filter_off = false;
    m_context.run_single_threaded = false;
  }

  /** Default deconstructor
   */
  ~dag_manager() {
    m_context.filter_off = true;
    clear();
  }

  /** Implementation to template funcs must be visible to link correctly
   *
   * Use this function to add a lambda function onto a parent.
   *
   * @param _id The node's name for later referencing
   * @param _new_filter The lambda function to run fromt he parent
   * @param _onto The node ID of the parent to attach the lambda function on to.
   */
  template <typename In, typename Out>
  void add_node(IDType _id, dag_node<In, Out> *_new_filter, IDType _onto) {
    if (_new_filter != nullptr) {
      for (auto t = m_all_dags.cbegin(); t != m_all_dags.cend(); t++) {
        if ((*t)->dag_contains(_onto) || (*t)->get_id() == _onto) {
          dag<In, IDType> *tptr = static_cast<dag<In, IDType> *>(*t);
          tptr->add_filter(_id, _new_filter, _onto);
        }
      }
    }
  }

  /** Containment function for checking presence
   *
   * This function simply looks for an ID on all dags
   *
   * @param _id The ID to check for
   *
   * @return Whether or not any of the dags contain the ID provided.
   */
  bool manager_contains_id(IDType _id) {
    for (auto t = m_all_dags.cbegin(); t != m_all_dags.cend(); t++) {
      if ((*t)->dag_contains(_id) || (*t)->get_id() == _id) return true;
    }
    return false;
  }

  /** Indentation delimiter
   *
   * Setter for the print function to set the identation spaces between nodes
   * when printing the dags.
   *
   * @param _new_indent_str The new indentation string. These are concatenated.
   */
  void set_indention_string(string _new_indent_str) {
    m_context.indent_str = _new_indent_str;
  }

  /** Set a logging stream to print to.
   *
   * Logging is complicated. As long as the user conforms to a std::ostream
   * then they can override the printout stream to print elsewhere like a
   * file or another interleaved stream instead of std::cout.
   *
   * @param _new_stream The new stream to print to for debug messaging.
   */
  void set_logging_stream(std::ostream *_new_stream) {
    m_context.log = _new_stream;
  }

  /** Sets whether to run the DAGs on the same thread
   *
   * Setter that will let all new nodes be created on separate threads
   * or on the same thread as it was called on.
   *
   * This has unsafe behavior when set during a single dags construction. This
   * can be set between each dags creation, however.
   *
   * @param _is_single_threaded Whether or not new nodes and source nodes are on
   * same node.
   */
  void run_single_threaded(bool _is_single_threaded) {
    m_context.run_single_threaded = _is_single_threaded;
  }

  /** Starts a new DAG with a given source of data out
   *
   * This begins a new dag (tree) that must generate output data sequentially.
   *
   * @param _id The DAGs name
   * @param _new_filter The generator function.
   * @param _startImmediately Whether or not to begin generating data on a loop
   * immediately.
   *
   * @return Returns the created DAG if the user wants it. Otherwise, NULL if
   * something fails.
   */
  template <typename Out>
  dag<Out, IDType> *add_dag(IDType _id, dag_source<Out> *_new_filter,
                            bool _startImmediately) {
    if (_new_filter != nullptr) {
      dag<Out, IDType> *t =
          new dag<Out, IDType>(_id, _new_filter, m_context, _startImmediately);
      m_all_dags.push_back(t);
      return t;
    }
    return nullptr;
  }

  /** Print all of the trees for verification purposes
   *
   * Simply prints all of the dags and their nodes to the given output stream.
   * Defaults to std::cout.
   */
  void print_all_dags() {
    *m_context.log << std::endl;
    *m_context.log << "------------DAG Forest-----------\n";
    for (auto t = m_all_dags.cbegin(); t != m_all_dags.cend(); t++)
      (*t)->print();
    *m_context.log << "---------------------------------\n";
  }

  /** Stops all of the DAGs from generating data out
   *
   * Stops all of the DAGs from generating data out. Fair warning, sometimes
   * it takes a second for the nodes to complete.
   */
  void stahp() { m_context.filter_off = true; }

  /** Clears all of the DAGs out of the "forest" of DAGs.
   *
   * This doesn't stop the nodes, this simply clears out the tracked DAGs so if
   * you need to stop the DAGs and maintain the pointers manually, feel free to
   * do so.
   */
  void clear() {
    for (auto t = m_all_dags.begin(); t != m_all_dags.end(); t++) delete *t;
    m_all_dags.clear();
  }
};
}  // namespace fn_dag
