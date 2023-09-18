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
  private:
    _dag_context m_context;
  public: 
    dag_manager() {
      m_context.filter_off = false;
      m_context.run_single_threaded = false;
    }
    ~dag_manager() {
      clear();
    }
    vector<_dag_base<IDType> *> m_all_dags;
    
    //implementation to template funcs must be visible to link correctly
    template <typename In, typename Out> 
    void add_node(IDType _id, dag_node<In,Out> *_new_filter, IDType _onto) {
      if(_new_filter != nullptr) {
        for(auto t = m_all_dags.cbegin();t != m_all_dags.cend();t++) {
          if((*t)->dag_contains(_onto) || (*t)->get_id() == _onto) {
            dag<In, IDType> *tptr = static_cast<dag<In, IDType>*>(*t);
            tptr->add_filter(_id, _new_filter, _onto);
          }
        }
      }
    }
 
    bool manager_contains_id(IDType _parent_id) {
      for(auto t = m_all_dags.cbegin();t != m_all_dags.cend();t++) {
        if((*t)->dag_contains(_parent_id) || (*t)->get_id() == _parent_id)
          return true;
      }
      return false;
    }

    void set_indention_string(string _new_indent_str) {
      m_context.indent_str = _new_indent_str;
    }

    void set_logging_stream(std::ostream *_new_stream) {
      m_context.log = _new_stream;
    }

    void run_single_threaded(bool _is_single_threaded) {
      m_context.run_single_threaded = _is_single_threaded;
    }
    

    template <typename Out> 
    dag<Out, IDType> *add_dag(IDType _id, dag_source<Out> *_new_filter, bool _startImmediately) {
      if(_new_filter != nullptr) {
        dag<Out, IDType> *t = new dag<Out, IDType>(_id, _new_filter, m_context, _startImmediately);
        m_all_dags.push_back(t);
        return t;
      }
      return nullptr;
    }

  void print_all_dags() {
      *m_context.log << std::endl;
      *m_context.log << "------------DAG Forest-----------\n";
      for(auto t = m_all_dags.cbegin();t != m_all_dags.cend();t++)
        (*t)->print();  
      *m_context.log << "---------------------------------\n";
    }
    
    void stahp() {
      m_context.filter_off = true;
    }
    
    void clear() {
      for(auto t = m_all_dags.begin();t != m_all_dags.end();t++)
        delete *t;
      m_all_dags.clear();
    }


  };
}
