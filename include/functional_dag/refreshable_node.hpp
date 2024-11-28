
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
  