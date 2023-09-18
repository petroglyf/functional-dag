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

#include<functional>

#include<functional_dag/dag_interface.hpp>

namespace fn_dag {
  template<typename Out>
  class __dag_source : public dag_source<Out> {
  public:
    std::function<Out *()> m_generator;

    __dag_source(std::function<Out *()> _generator) : m_generator(_generator) {}
    ~__dag_source() {}
    Out *update() {return m_generator();}; 
  };

  template<typename In, typename Out>
  class __dag_node : public dag_node<In, Out> {
  public:
    std::function<Out *(const In *)> m_update;

    __dag_node(std::function<Out *(const In *)> _update) : m_update(_update) {}
    ~__dag_node() {}
    Out *update(const In *data) {return m_update(data);}; 
  };

  template<typename Out>
  dag_source<Out> *fn_source(std::function<Out *()> _run_fn) {
    return new __dag_source(_run_fn);
  }

  template<typename In, typename Out>
  dag_node<In, Out> *fn_call(std::function<Out *(const In *)> _run_fn) {
    return new __dag_node(_run_fn);
  }

}
