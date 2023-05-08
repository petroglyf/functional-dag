/**
 *   ___                 .___               
 *  |_  \              __| _/____     ____  
 *   /   \    ______  / __ |\__  \   / ___\ 
 *  / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 * /_/  \__\         \____ |(____  /\___  / 
 *                        \/     \//_____/   
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
