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
    vector< _abstract_internal_dag_node<Type, IDType> *> m_children;

    void add_node(_abstract_internal_dag_node<Type, IDType> *_new_node) {
      m_children.push_back(_new_node);
    }

  public:
    dag_fanout_node(const _dag_context &_context) : 
                                              g_context(_context), 
                                              m_children() {}
    ~dag_fanout_node() {
      for(auto internal_dag : m_children)
        delete internal_dag;
      m_children.clear();
    }
    
    void fan_out(Type *_data) {
      if(!g_context.run_single_threaded) {
        std::vector<thread> child_threads;

        for(auto it : m_children)
          child_threads.push_back(thread(&fn_dag::_abstract_internal_dag_node<Type, IDType>::run_filter, it, std::ref(_data)));

        for(int i = 0;i < child_threads.size();i++)
          child_threads[i].join();
      } else
        for(auto it : m_children)
          it->run_filter(_data);

      delete _data;
    }

    void print(string _indent) {
      string plus = _indent+g_context.indent_str;
      for(auto child : m_children) {
        *g_context.log << _indent <<  "->";
        child->print(plus);
      }
    }

    template <typename In, typename Out>
    bool add_node_to_subdag(_internal_dag_node<In,Out,IDType> *_node_to_add, 
                            const IDType _onto, 
                            const IDType _parent_id) {
      if(_onto == _parent_id) {
        add_node((_abstract_internal_dag_node<In, IDType>*)_node_to_add);
        return true;
      } else {
        for(auto child = m_children.cbegin();child != m_children.cend();child++) {
          auto *internal_child = static_cast<_internal_dag_node<In, Type, IDType> *>(*child);
          fn_dag::dag_fanout_node<Type, IDType> *fanout_node = internal_child->m_child;
          if(fanout_node->add_node_to_subdag(_node_to_add, _onto, (*child)->get_id()))
            return true;
        }
      }
      
      return false;
    }
  };
}
