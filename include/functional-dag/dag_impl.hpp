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

/**************************************************
 * Trees work like this: 
 *
 *  _dag_base
 *  |-Tree
 *  |--SpreadAroundTreeNode -> single data handle here
 *  |---*dag_node -> Filter, data not locked, thread waiting here
 *  |-----SpreadAroundTreeNode
 *  |------*dag_node -> Filter
 */
namespace fn_dag {
  extern bool __g_filter_off;
  extern bool __g_run_single_threaded;

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
  public:
    const IDType m_id;
    dag_source<OriginType> *m_source;
    
    dag_fanout_node<OriginType, IDType> m_children;
    unordered_set<IDType> m_children_ids;
    
    dag(IDType _id, dag_source<OriginType> *_lsource, bool _startThread) :
          m_children_ids(), m_source(_lsource), m_id(_id), m_children() {
      if(_startThread) {
        startSource();
      }	      
    }

    ~dag() {
      if(m_thread.joinable())
        m_thread.join();
      delete m_source;
    }

    IDType get_id() {
      return m_id;
    }

    void manualPump(OriginType *rawDat) {
      m_children.spreadAround(rawDat);
    }


    bool dag_contains(IDType _id) {
      return m_children_ids.count(_id) > 0;
    }
    
    template<typename In, typename Out> 
    void addFilter(IDType _newID, dag_node<In, Out> *newFilter, IDType onNode) {
      _internal_dag_node<In, Out, IDType> *new_node;
      new_node = new _internal_dag_node<In, Out, IDType>(_newID, newFilter);
      m_children_ids.insert(_newID);
      findAndAdd(new_node, onNode, this->get_id(), &m_children);
    }

    void print() {
      std::cout << "->" << this->m_id << std::endl;
      m_children.print("  ");
    }

    void push_once() {
      if(!fn_dag::__g_run_single_threaded) {
        // std::packaged_task<void()> task(std::bind(&dag<OriginType,IDType>::__push_once, this));
        // // std::future<int> f1 = task.get_future();  // get a future
        // std::thread tmp_thread(std::thread(std::move(task)));
        // m_thread.swap(tmp_thread); // launch on a thread
        __push_once();
      } else {
        __push_once();
      }
      
    }

  private:
    thread m_thread;
    void __push_once() {
      OriginType *dat = m_source->update();
      if(dat != nullptr) {
        // std::cout << "dat not null\n";
        m_children.spread_around(dat);
      }
    }

    
    void startsrc() {
      std::cout<< " startsrc\n";
      while(!__g_filter_off) {
	      push_once();
      }
    }
    
    void startSource() {
      m_thread = std::thread(&dag::startsrc, this);
    }
    
    template <typename In, typename Out, typename CurrOut>
    bool findAndAdd(_internal_dag_node<In,Out,IDType> *addNode, const IDType onto, const IDType _parent_id, dag_fanout_node<CurrOut, IDType> *conn) {
      if(onto == _parent_id) {
        conn->m_children.push_back((_abstract_internal_dag_node<In, IDType>*)addNode);
        return true;
      } else 
      for(auto child = conn->m_children.cbegin();child != conn->m_children.cend();child++) {
        _internal_dag_node<In, CurrOut, IDType> *tn;
        tn = static_cast<_internal_dag_node<In, CurrOut, IDType> *>(*child);
        fn_dag::dag_fanout_node<CurrOut, IDType> *satn = tn->m_child;
        if(findAndAdd(addNode, onto, (*child)->get_id(), satn))
          return true;
      }
      return false;  
    }
  };
};
