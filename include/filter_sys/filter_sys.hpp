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

#include "functional_dag/dag_impl.hpp"
#include "functional_dag/dag_node_impl.hpp"
#include "functional_dag/dag_interface.hpp"

namespace fn_dag {

  template <typename IDType> 
  class dag_manager {
    public: 
    dag_manager() {
      fn_dag::__g_filter_off = false;
      fn_dag::__g_run_single_threaded = false;
    }
    ~dag_manager() {
      clearAllTrees();
    }
    vector<_dag_base<IDType> *> m_allTrees;
    
    //implementation to template funcs must be visible to link correctly
    template <typename In, typename Out> 
    void add_node(IDType _id, dag_node<In,Out> *newFilter, IDType _onto) {
      std::cout << "Adding node2\n";
      if(newFilter != nullptr) {
        std::cout << "node not null\n";
        for(auto t = m_allTrees.cbegin();t != m_allTrees.cend();t++) {
          std::cout << "is below? " << (*t)->get_id() << " onto " << _onto << std::endl;
          if((*t)->dag_contains(_onto) || (*t)->get_id() == _onto) {
            std::cout << "found\n";
            dag<In, IDType> *tptr = static_cast<dag<In, IDType>*>(*t);
            tptr->addFilter(_id, newFilter, _onto);
          }
        }
      }
    }
 
    bool manager_contains_id(IDType _parent_id) {
      for(auto t = m_allTrees.cbegin();t != m_allTrees.cend();t++) {
        std::cout << "t id? " << (*t)->get_id() << std::endl;
        if((*t)->dag_contains(_parent_id) || (*t)->get_id() == _parent_id)
          return true;
      }
      return false;
    }
    

    template <typename Out> 
    dag<Out, IDType> *add_dag(IDType _id, dag_source<Out> *_new_filter, bool _startImmediately) {
      if(_new_filter != nullptr) {
        dag<Out, IDType> *t = new dag<Out, IDType>(_id, _new_filter, _startImmediately);
        m_allTrees.push_back(t);
        return t;
      }
      return nullptr;
    }

  void printAllTrees() {
      std::cout << std::endl;
      std::cout << "-----------Filter Tree-----------\n";
      for(auto t = m_allTrees.cbegin();t != m_allTrees.cend();t++)
        (*t)->print();  
      std::cout << "---------------------------------\n";
    }
    
    void Stahp() {
      fn_dag::__g_filter_off = true;
    }
    
    void clearAllTrees() {
      for(auto t = m_allTrees.begin();t != m_allTrees.end();t++)
        delete *t;
      m_allTrees.clear();
    }


  };
}