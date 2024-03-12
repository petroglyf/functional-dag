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

#include<functional>

#include<functional_dag/dag_interface.hpp>

namespace fn_dag {
  /** Internal structure to support a generator function
  */
  template<typename Out>
  class __dag_source : public dag_source<Out> {
  public:
    std::function<Out *()> m_generator;             // Generator lambda function

    /** Default constructor 
     * @param _generator A lambda function to call repeatedly.
    */
    __dag_source(std::function<Out *()> _generator) : m_generator(_generator) {}

    /** Default deconstructor */
    ~__dag_source() {}

    /** Overloaded function to call the generator function. 
     * @return Output data from the lambda function
    */
    Out *update() {return m_generator();}; 
  };

  /** Internal structure to support a mapping function
  */
  template<typename In, typename Out>
  class __dag_node : public dag_node<In, Out> {
  public:
    std::function<Out *(const In *)> m_update;      // Mapping lambda function

    /** Default constructor 
     * @param _update A lambda function to call repeatedly on input data
    */
    __dag_node(std::function<Out *(const In *)> _update) : m_update(_update) {}

    /** Default deconstructor */ 
    ~__dag_node() {}

    /** Overloaded function to call the mapping function. 
     * @param _data Input data to the lambda function
     * @return Output data from the lambda function
    */
    Out *update(const In *_data) {return m_update(_data);}; 
  };

  /** A wrapper function that constructs a generator wrapper for your generator function 
   * 
   * Use this function if you don't need any state to maintain in your source node. If your
   * generator is stateless, this can be a helpful function to use and wrap your lambda 
   * around. 
   * 
   * @param _run_fn A lambda function that outputs *Out* typed data when called
   * @return A wrapped, compatible, source node for the dag tree.
  */
  template<typename Out>
  dag_source<Out> *fn_source(std::function<Out *()> _run_fn) {
    return new __dag_source(_run_fn);
  }

  /** A wrapper function that constructs a mapping wrapper for your mapping function 
   * 
   * Use this function if you don't need any state to maintain in your source node. If your
   * mapping funciton is stateless, this can be a helpful function to use and wrap your lambda 
   * around. 
   * 
   * @param _run_fn A lambda function that outputs *Out* typed data when called with *In* type data.
   * @return A wrapped, compatible, dag node for the dag tree.
  */
  template<typename In, typename Out>
  dag_node<In, Out> *fn_call(std::function<Out *(const In *)> _run_fn) {
    return new __dag_node(_run_fn);
  }

}
