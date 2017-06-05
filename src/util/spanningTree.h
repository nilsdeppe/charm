/**
 * Author: jjgalvez@illinois.edu (Juan Galvez)
 */
#ifndef __SPANNING_TREE_H_
#define __SPANNING_TREE_H_

#if defined(__cplusplus)
extern "C" {
#endif

/// C API to ST_RecursivePartition_getTreeInfo
void get_topo_tree_nbs(int root, int *parent, int *child_count, int **children);

#if defined(__cplusplus)
}

#include "charm++.h"
#include <vector>

/**
 * Abstract class (interface) to generate a spanning tree from a set of pes or logical
 * (Charm++) nodes. These will be referred to simply as nodes, physical nodes will
 * be referred to as phynodes.
 */
template <typename Iterator>
class SpanningTreeGenerator {
public:
  /**
   * Given a range of nodes (with root in first position), calculates the children
   * of root (and their corresponding subtree).
   * Tree algorithm will reorganize nodes inside the range, so that they are grouped by
   * subtree.
   * Use below methods to access the results.
   *
   * \param start Iterator to beginning of node range
   * \param end Iterator to end of node range
   * \param maxBranches Max number of children the root should have
   * \note Each algorithm can interpret this differently.
   *
   * \return number of children of root generated by algorithm
   */
  virtual int buildSpanningTree(Iterator start, Iterator end,
                                unsigned int maxBranches) = 0;

  /// return number of nodes in generated subtree (includes root of subtree)
  virtual int subtreeSize(int subtree) = 0;

  /// return Iterator to first node in generated subtree (i.e. root of subtree)
  virtual Iterator begin(int subtree) = 0;

  /// return Iterator to end of generated subtree
  virtual Iterator end(int subtree) = 0;

};

// ------------- ST_RecursivePartition -------------

class TopoManager;

/**
 * obtain TreeInfo (parent and children) of CkMyNode() for tree rooted at
 * specified node using ST_RecursivePartition. Tree is assumed to cover all nodes.
 * This function allocates and caches the TreeInfo structure.
 */
CmiSpanningTreeInfo *ST_RecursivePartition_getTreeInfo(int root);

/**
 * This strategy is phynode aware, and can form a tree of pes or logical nodes.
 * Will benefit from topology information (coordinates of hosts in the machine,
 * and distance between hosts), but can be used without topology information
 * (and will still be phynode aware).
 *
 * Works for any N-d mesh/torus, including non-contiguous allocations and holes in
 * allocation (e.g. Blue Waters).
 *
 * Algorithm complexity is O(n) where n is number of nodes.
 *
 * Inside a phynode, there will only be one root node, and currently every other
 * node in that phynode is a direct descendant of it (this can be easily changed).
 * Edges between phynodes are only between their corresponding root nodes (as a
 * result there will only be one edge between phynodes).
 */
template <typename Iterator>
class ST_RecursivePartition : public SpanningTreeGenerator<Iterator> {
public:

  /**
   * \param nodeTree true if forming tree of nodes, false if tree of pes.
   *
   * \param preSorted true if nodes will be provided grouped by phynode. Allows
   * using slightly more efficient implementation.
   */
  ST_RecursivePartition(bool nodeTree=true, bool preSorted=false);

  /**
   * Will reorder node range so that nodes are grouped by subtree, *and* by phynode.
   *
   * \param maxBranches Max number of children in different phynode as root
   */
  virtual int buildSpanningTree(Iterator start, Iterator end, unsigned int maxBranches);

  inline virtual int subtreeSize(int subtree) {
#if CMK_ERROR_CHECKING
    return std::distance(children.at(subtree), children.at(subtree+1));
#else
    return std::distance(children[subtree], children[subtree+1]);
#endif
  }

  inline virtual Iterator begin(int subtree) {
#if CMK_ERROR_CHECKING
    return children.at(subtree);
#else
    return children[subtree];
#endif
  }

  inline virtual Iterator end(int subtree) {
#if CMK_ERROR_CHECKING
    return children.at(subtree+1);
#else
    return children[subtree+1];
#endif
  }

private:

  class PhyNode;
  class PhyNodeCompare;

  void initPhyNodes(Iterator start, Iterator end,
                    std::vector<PhyNode> &phynodes) const;
  void build(std::vector<PhyNode*> &phyNodes, Iterator start, unsigned int maxBranches);
  void partition(std::vector<PhyNode*> &nodes, int start, int end,
                 int numPartitions, std::vector<int> &children) const;
  void chooseSubtreeRoots(std::vector<PhyNode*> &phyNodes,
                          std::vector<int> &children) const;
  void bisect(std::vector<PhyNode*> &nodes, int start, int end,
              int numPartitions, std::vector<int> &children) const;
  void trisect(std::vector<PhyNode*> &nodes, int start, int end,
              int numPartitions, std::vector<int> &children) const;
  int maxSpreadDimension(std::vector<PhyNode*> &nodes, int start, int end) const;
#if XE6_TOPOLOGY
  void translateCoordinates(std::vector<PhyNode> &nodes) const;
#endif

  std::vector<Iterator> children;
  bool nodeTree;
  bool preSorted;
  TopoManager *tmgr;
};

#endif
#endif
