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

#include <vector>
#include <map>
#include <iostream>
#include <unordered_set>
#include "functional_dag/dag_interface.hpp"
#include "functional_dag/dag_fanout_impl.hpp"

namespace fn_dag {

  template<typename IDType>
  class _dag_base {
  public:
    virtual IDType get_id() = 0;
    virtual ~_dag_base() = default;
    virtual bool dag_contains(IDType _id) = 0;
    virtual void print() = 0;
    virtual void push_once() = 0;
  };

  template<typename OriginType, typename IDType>
  class dag : public _dag_base<IDType> {
  private:
    const IDType m_id;
    dag_source<OriginType> *m_source;
    
    dag_fanout_node<OriginType, IDType> m_children;
    unordered_set<IDType> m_children_ids;
    const _dag_context &g_context;
  public:
    dag(IDType _id, 
        dag_source<OriginType> *_lsource, 
        const _dag_context &_context, 
        bool _startThread) :
                  m_children_ids(), 
                  m_source(_lsource), 
                  m_id(_id), 
                  m_children(_context), 
                  g_context(_context) {
      if(_startThread)
        m_thread = std::thread(&dag::start_source, this);
    }

    ~dag() {
      if(m_thread.joinable())
        m_thread.join();
      delete m_source;
    }

    IDType get_id() {
      return m_id;
    }

    void manual_pump(OriginType *_raw_dat) {
      m_children.fan_out(_raw_dat);
    }


    bool dag_contains(IDType _id) {
      return m_children_ids.count(_id) > 0;
    }
    
    template<typename In, typename Out> 
    void add_filter(IDType _newID, dag_node<In, Out> *_new_filter, IDType _on_node) {
      _internal_dag_node<In, Out, IDType> *new_node;
      new_node = new _internal_dag_node<In, Out, IDType>(_newID, _new_filter, g_context);
      m_children_ids.insert(_newID);
      m_children.add_node_to_subdag(new_node, _on_node, m_id);
    }

    void print() {
      *g_context.log << "->" << m_id << std::endl;
      m_children.print(g_context.indent_str);
    }

    void push_once() {
      OriginType *dat = m_source->update();
      if(dat != nullptr)
        m_children.fan_out(dat);
    }

  private:
    thread m_thread;
    
    void start_source() {
      while(!g_context.filter_off)
	      push_once();
    }
  };
};
