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

#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include "functional_dag/dag_interface.hpp"
#include "functional_dag/impl/dag_fanout_impl.hpp"
#include "functional_dag/core/dag_utils.hpp"

namespace fn_dag {
  using namespace std;

  template<typename Out, typename IDType> class dag_fanout_node;

  template <class Type, class IDType>
  class _abstract_internal_dag_node {
  public:
    virtual ~_abstract_internal_dag_node() = default;
    virtual IDType get_id() = 0;
    virtual void run_filter(const Type *_data) = 0;
    virtual void print(string _plus) = 0;
  };

  template<typename In, typename Out, typename IDType>
  class _internal_dag_node : public _abstract_internal_dag_node<In, IDType> {
    friend class dag_fanout_node<In, IDType>;
    
  private:
    dag_node<In,Out> *m_node_hook;
    const IDType m_node_id;
    dag_fanout_node<Out, IDType> *m_child;
    const fn_dag::_dag_context &g_context;
    
  public:
    _internal_dag_node(IDType _node_id, dag_node<In,Out> *_node, const fn_dag::_dag_context &_context) :
      m_node_id(_node_id),
      m_node_hook(_node),
      g_context(_context),
      m_child(new dag_fanout_node<Out, IDType>(_context)){}

    ~_internal_dag_node() {
      delete m_child;
      delete m_node_hook;
    }

    void run_filter(const In *_data) {
      Out *data_out = m_node_hook->update(_data);
      if(!g_context.filter_off && data_out != nullptr)
        m_child->fan_out(data_out);
    }

    void print(string _indent_str) {
      *g_context.log << m_node_id << std::endl;
      m_child->print(_indent_str);
    }

    IDType get_id() {
      return m_node_id;
    }
  };
}
