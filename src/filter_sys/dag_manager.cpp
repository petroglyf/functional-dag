#include "filter_sys/dag_impl.hpp"
#include "filter_sys/dag_node_impl.hpp"
#include "filter_sys/dag_interface.hpp"

namespace fn_dag {
  bool __g_filter_off = false;
  bool __g_run_single_threaded = false;
  std::string __g_indent_str = "  ";

    
  void set_indention_string(string _new_indent_str) {
    __g_indent_str = _new_indent_str;
  }
  
  /** Not ready to port this yet.
      template<class In, class Out>
      class TreeSplitNode : public dag_node<In, std::map<std::string, Out> > //, FilterType> 
      {
      public:
      TreeSplitNode(std::string name, Filter<In, std::map<std::string, Out> > node, 
      std::vector<std::string> childrenNames);
      ~TreeSplitNode();
	    
      std::map<std::string, SpreadAroundTreeNode<Out> *> *children;
      
      void run(const In *data);
      void runFilter(const In *data);
      };
      
      template<class In, class Out>
      TreeSplitNode<In, Out>::TreeSplitNode(std::string name, Filter<In, std::map<std::string, Out> > node, 
      std::vector<std::string> childrenNames) :
      dag_node<In, std::map<std::string, Out> >(name, node), 
      children(NULL) {
      children = new std::map<std::string, SpreadAroundTreeNode<Out> >();
      
      for(auto childName = childrenNames.cbegin(); childName != childrenNames.cend();childName++)
      children->put(*childName, new SpreadAroundTreeNode<Out>());
      }
      
      template<class In, class Out>
      TreeSplitNode<In, Out>::~TreeSplitNode() {
      for(auto t = children.begin();t != children.end();t++) 
      delete t->second;
      delete children;
      } 
      
      
      template<class In, class Out>
      void TreeSplitNode<In, Out>::run(const In *data) {
      std::map<std::string, Out> dout = this->theNode.update(data);
      this->child.spreadAround(dout);
      for(auto hasOutUpdate = dout.cbegin();hasOutUpdate != dout.cend();hasOutUpdate++)
      children.get(*hasOutUpdate.first).spreadAround(dout.get(*hasOutUpdate.first));
      }    
      
      template<class In, class Out>
      void TreeSplitNode<In, Out>::runFilter(const In *data) {
      //TODO: FiX THIS THE WAY LIKE ABOVE
      if(FilterTree::runSingleThreaded)
      run(data);
      else
      std::thread t(TreeSplitNode::run, data);
      }
  */
        
  
    /*  template<class In, class Out> 
	void addMultiOutFilter(std::string newName, Filter<In, std::map<std::string, Out>> newFilter, std::string onNode,
	std::vector<std::string> names) {
	TreeSplitNode<In, Out> *nnode = new TreeSplitNode<In, Out>(newName, newFilter, names);
	filtermap.put(newName, nnode->child);
	for(auto name = names.cbegin();name != names.cend();name++)
	filtermap.put(name, nnode->children->get(name));
	findAndAdd(nnode, onNode, name, children);
	//if(newFilter instanceof InteractiveFilter) {
	//  refreshables.add((InteractiveFilter)newFilter);
	//}
	}*/
    
    
    /*void refresh() {
     //////////
     //Handle tree root
     if(source instanceof InteractiveFilter) {
     ((InteractiveFilter)source).refreshProxy();
     }
     
     /////////
     //Handle all nodes in tree (iterate flattened list)
     for(InteractiveFilter ifs : refreshables) {
     ifs.refreshProxy();
     }
     }*/

    
  
  
  /*

    public static class FilterTreeManual<RawIn> {
    @SuppressWarnings("rawtypes")
    private Tree innerTree;
    public FilterTreeManual(FilterSource<RawIn> first) {
    innerTree = new FilterTree.Tree<RawIn>("customTree", first, false);
    }
    @SuppressWarnings({ "rawtypes", "unchecked" })
    public <In, Out> void addFilter(String name, Filter<In,Out> newFilter, String onto) {
    if(innerTree.filtermap.containsKey(onto) || innerTree.name.equals(onto))
    innerTree.addFilter(name, newFilter, onto);
    }
    
    @SuppressWarnings({ "rawtypes", "unchecked" })
    public <In, Out> void addMultiOutFilter(String name, Filter<In,Out> newFilter, String onto, List<String> allOuts) {
    if(innerTree.filtermap.containsKey(onto) || innerTree.name.equals(onto))
    innerTree.addMultiOutFilter(name, newFilter, onto, allOuts);
    }

    public void printTree() {
    innerTree.print();
    }
    @SuppressWarnings("unchecked")
    public void rawPump(RawIn b) {
    innerTree.manualPump(b);
    }
    }
  */
  
  
  /*template <class In, class Out> 
    void FilterTree::addMultiOutFilter(std::string name, Filter<In,Out> *newFilter, std::string onto, 
    std::vector<std::string> allOuts) {
    for(auto t = allTrees.cbegin();t != allTrees.cend();t++) {
    if((*t)->filter_map_contains(onto) || (*t)->name.compare(onto) == 0)
    ((Tree<In>*)(*t))->addMultiOutFilter(name, newFilter, onto, allOuts);
    }
    }
  */
  
   
  // template<class Out>
  // void addFilterSource(std::string name, FilterSource<Out> *newFilter, bool startImmediately) {
  // // void addFilterSource(std::string name, bool startImmediately) {
  // }
  
  
}

