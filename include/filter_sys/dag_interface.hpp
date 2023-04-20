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

namespace fn_dag {
  template<typename Out>
  class dag_source {
  public:

    virtual ~dag_source() = default;
    virtual Out *update() = 0; 
  };

  template<typename In, typename Out>
  class dag_node {
  public:

    virtual ~dag_node() = default;
    virtual Out *update(const In *data) = 0;
  };
}
