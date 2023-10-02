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

namespace fn_dag {
  /** Interface for all external generator lambdas
   * 
   * All user functions must implement the update() function that returns a given 
   * type specified by the user. 
  */
  template<typename Out>
  class dag_source {
  public:
    /** Default constructor
    */
    virtual ~dag_source() = default;

    /** Generator function to implement by the user.
     * @return New data that was just generated
    */
    virtual Out *update() = 0; 
  };

  /** Interface for all external "mapping" lambdas
   * 
   * All nodes simply "translate" input data to output data. 
   * 
   * Note: Do not try to manage the output data. Since it is a pointer, 
   * it is passed between nodes and then deleted by the dag manager.
  */
  template<typename In, typename Out>
  class dag_node {
  public:

    /** Default constructor 
    */
    virtual ~dag_node() = default;

    /** Translator function 
     * 
     * This function translates the input data to the output data. The pointer data
     * should be on the heap and should be allocated with *new*.
     * 
     * @param _data The data to use to generate the output data
     * @return Data out, just allocated on the heap with *new*. 
    */
    virtual Out *update(const In *_data) = 0;
  };
}
