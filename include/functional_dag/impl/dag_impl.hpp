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

#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

#include "functional_dag/dag_interface.hpp"
#include "functional_dag/impl/dag_fanout_impl.hpp"

namespace fn_dag {

/** Abstract pure virtual class for all DAGs
 *
 * Simply, this allows the dag manager to standardize the ID type at the
 * very least.
 */
template <typename IDType>
class _dag_base {
 public:
  /** Default constructor for cleanup */
  virtual ~_dag_base() = default;

  /** Check to see if the DAG contains a specific ID
   * @return Returns true if the dag contains the ID
   */
  virtual bool dag_contains(IDType _id) = 0;

  /** Simple print statement that recursively prints the nodes. */
  virtual void print() = 0;

  /** Gets the ID of the DAG itself
   * @return The ID of the DAG itself
   */
  virtual IDType get_id() = 0;

  /** Calls the source generator data and propagates it across the DAG once. */
  virtual void push_once() = 0;
};

/** The main DAG function that encapulates generation and mapping of the data
 * across the DAG.
 *
 * The DAG calls the source generator to generate some data. That data is then
 * propagated to it's children and across the entire DAG. The DAG stops
 * propagating across the children of any node of the DAG if the previous parent
 * returns nullptr. Otherwise, this runs either multithreaded or single threaded
 * and can be manually manipulated (i.e. skip the generator) by turning it off
 * and manually calling the *manual_pump* function.
 */
template <typename OriginType, typename IDType>
class dag : public _dag_base<IDType> {
 private:
  const IDType m_id;                 // The ID of the DAG itself
  dag_source<OriginType> *m_source;  // The source generator that creates data
  dag_fanout_node<OriginType, IDType>
      m_children;  // The children of the source to propagate data across
  unordered_set<IDType> m_children_ids;  // An optimization: a quick O(1) set
                                         // lookup of the children IDs
  const _dag_context
      &g_context;   // The shared state across all of the children of this node.
  thread m_thread;  // Thread to run on if this DAG runs multi-threaded.

 public:
  /** Constructor of the DAG. Ideally this is created in the manager but you can
   * create your own DAG if you'd like.
   *
   * @param _id The ID of the DAG itself
   * @param _lsource The main generator (source) of the DAG. Where data
   * originates
   * @param _context The context shared across the DAG
   * @param _startThread Whether or not to autostart calling the generator
   * function
   */
  dag(IDType _id, dag_source<OriginType> *_lsource,
      const _dag_context &_context, bool _startThread)
      : m_id(_id),
        m_source(_lsource),
        m_children(_context),
        m_children_ids(),
        g_context(_context) {
    if (_startThread) m_thread = std::thread(&dag::start_source, this);
  }

  /** Default deconstructor. Waits for children to stop before cleaning up. */
  ~dag() {
    if (m_thread.joinable()) m_thread.join();
    delete m_source;
  }

  /** Simple getter for the ID of the DAG itself
   *
   * Gets the ID of the DAG.
   *
   * @return ID of the DAG
   */
  IDType get_id() { return m_id; }

  /** Manually pumps some data across the children of the DAG
   *
   * This is a useful function for testing. It *skips* the generator and pushes
   * the test source data across the DAGs children.
   *
   * @param _raw_dat The raw data that the user provides.
   */
  void manual_pump(OriginType *_raw_dat) { m_children.fan_out(_raw_dat); }

  /** Checks whether this DAG contains a specific ID
   *
   * Given an ID, it will check the optimized hash set to see if the child
   * exists.
   *
   * @param _id The ID to lookup
   * @return Whether or not the DAG contains the ID.
   */
  bool dag_contains(IDType _id) { return m_children_ids.count(_id) > 0; }

  /** Adds a new function to the DAG.
   *
   * Since the generator already exists, this function recursively calls itself
   * with the parents nodes type and, in a strongly typed way, attaches a child
   * to the parent to process the data output by the parent node.
   *
   * @param _newID The ID of the new function
   * @param _new_filter The mapping function itself to pass along
   * @param _on_node The ID of the parent to attach the function to.
   */
  template <typename In, typename Out>
  void add_filter(IDType _newID, dag_node<In, Out> *_new_filter,
                  IDType _on_node) {
    _internal_dag_node<In, Out, IDType> *new_node;
    new_node =
        new _internal_dag_node<In, Out, IDType>(_newID, _new_filter, g_context);
    m_children_ids.insert(_newID);
    m_children.add_node_to_subdag(new_node, _on_node, m_id);
  }

  /** Simple print function to print the ID of this DAG and it's children. */
  void print() {
    *g_context.log << "->" << m_id << std::endl;
    m_children.print(g_context.indent_str);
  }

  /** Runs the generator and begins propagating the data to it's children
   *
   * This function can be called from the thread on a loop or called on a single
   * thread. This encapsulates a single pass across the DAG.
   */
  void push_once() {
    OriginType *dat = m_source->update();
    if (dat != nullptr) m_children.fan_out(dat);
  }

 private:
  /** Private function to run on a thread. Loops until asked to stop.
   *
   * This is the thread function. Runs until the DAG is asked to stop.
   */
  void start_source() {
    while (!g_context.filter_off) push_once();
  }
};
};  // namespace fn_dag
