/**
  *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * 
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
#include <filter_sys/dag_node_impl.hpp>
#include <filter_sys/dag_utils.hpp>

namespace fn_dag {
  template <typename Type, typename IDType>
  class dag_fanout_node {
  public:
    vector< _abstract_internal_dag_node<Type, IDType> *> m_children;

    dag_fanout_node() : m_children() {}
    ~dag_fanout_node() {
      for(auto internal_dag : m_children)
        delete internal_dag;
      m_children.clear();
    }
    
    void spread_around(Type *_data) {
      // std::cout << " spreading around\n";
      for(auto it : m_children) {
        std::cout << "Running child filter\n";
        it->runFilter(_data);
      }
    }

    void print(string _indent) {
      string plus = _indent+__g_indent_str;
      for(auto child : m_children) {
        std::cout << _indent <<  "->";
        child->print(plus);
      }
    }
  };
}
