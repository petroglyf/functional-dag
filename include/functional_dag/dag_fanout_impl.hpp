/**
 *   ___                 .___               
 *  |_  \              __| _/____     ____  
 *   /   \    ______  / __ |\__  \   / ___\ 
 *  / /\  \  /_____/ / /_/ | / __ \_/ /_/  >
 * /_/  \__\         \____ |(____  /\___  / 
 *                        \/     \//_____/   
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <future>
#include <thread>
#include <functional_dag/dag_node_impl.hpp>
#include <functional_dag/dag_utils.hpp>

namespace fn_dag {

  /**
   * @brief Internal node to take data from parent and run children.
   */
  template <typename Type, typename IDType>
  class dag_fanout_node {
  private:
    
    const fn_dag::_dag_context &g_context;
  public:
    vector< _abstract_internal_dag_node<Type, IDType> *> m_children; //TODO: Make this private
    
    dag_fanout_node(const _dag_context &context_) : 
                                              g_context(context_), 
                                              m_children() {}
    ~dag_fanout_node() {
      for(auto internal_dag : m_children)
        delete internal_dag;
      m_children.clear();
    }
    
    void fan_out(Type *_data) {
      for(auto it : m_children)
        it->runFilter(_data);
    }

    void print(string _indent) {
      string plus = _indent+g_context.indent_str;
      for(auto child : m_children) {
        *g_context.log << _indent <<  "->";
        child->print(plus);
      }
    }

    void add_node(_abstract_internal_dag_node<Type, IDType> *_new_node) {
      m_children.push_back(_new_node);
    }

    void find_and_add_to_children(_abstract_internal_dag_node<Type, IDType> *_new_node) {
      
    }
  };
}
