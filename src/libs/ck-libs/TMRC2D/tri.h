// Triangular Mesh Refinement Framework - 2D (TMR)
// Created by: Terry L. Wilmarth
#ifndef TRI_H
#define TRI_H

#include <vector.h>
#include "charm++.h"
#include "tcharm.h"
#include "charm-api.h"
#include "ref.h"
#include "node.h"
#include "edge.h"
#include "element.h"
#include "refine.decl.h"
#include "messages.h"

// ------------------------ Global Read-only Data ---------------------------
extern CProxy_chunk mesh;
class chunk;
CtvExtern(chunk *, _refineChunk);


/**
 * The user inherits from this class to receive "split" calls,
 * and to be informed when the refinement is complete.
 */
class refineClient {
public:
  virtual ~refineClient() {}

  /**
   * This triangle of our chunk is being split along this edge.
   *
   * For our purposes, edges are numbered 0 (connecting nodes 0 and 1), 
   * 1 (connecting 1 and 2), and 2 (connecting 2 and 0).
   * 
   * Taking as A and B the (triangle-order) nodes of the splitting edge:
   *
   *     ____ A          ____ A   
   *   /      |        /      |  
   *  C       |  =>   C ----- D  <- new node
   *   \      |        \      |   
   *     ---- B          ---- B 
   *
   *   The original triangle should be shrunk to ADC; while a 
   * new triangle should be inserted at DBC.
   *
   *   The new node D's location should equal A*(1-frac)+B*frac.
   * For a simple splitter, frac will always be 0.5.
   *
   *   If nodes A and B are shared with some other processor,
   * that processor will also receive a "split" call for the
   * same edge.  If nodes A and B are shared by some other local
   * triangle, that triangle will also receive a "split" call
   * for the same edge.  
   *
   * Client's responsibilities:
   *   -Add the new node D.  Since both sides of a shared edge
   *      will receive a "split" call, you must ensure the node is
   *      not added twice.
   *   -Update connectivity for source triangle
   *   -Add new triangle DBC.
   */
  virtual void split(int triNo,int edgeOfTri,int movingNode,double frac) =0;
  virtual void split(int triNo,int edgeOfTri,int movingNode,double frac,int flag) =0;

};

class refineResults; //Used by refinement API to store intermediate results

// ---------------------------- Chare Arrays -------------------------------
class chunk : public TCharmClient1D {
  // current sizes of arrays allocated for the mesh
  int sizeElements, sizeEdges, sizeNodes;

  void setupThreadPrivate(CthThread forThread) {
    CtvAccessOther(forThread, _refineChunk) = this;
  }
  // information about connectivity and location of ghost elements
  int *conn, *gid, additions;
  
  // debug_counter is used to print successive snapshots of the chunk
  // and match them up to other chunk snapshots; refineInProgress
  // flags that the refinement loop is active; modified flags that a
  // target area for some element on this chunk has been modified
  int debug_counter, refineInProgress, coarsenInProgress, modified;

  // meshLock is used to lock the mesh for expansion; if meshlock is
  // zero, the mesh can be either accessed or locked; accesses to the
  // mesh (by a chunk method) decrement the lock, and when the
  // accesses are complete, the lock is incremented; when an expansion
  // of the mesh is required, the meshExpandFlag is set, indicating
  // that no more accesses will be allowed to the mesh until the
  // adjuster gets control and completes the expansion; when the
  // adjuster gets control, it sets meshLock to 1 and when it is
  // finished, it resets both variables to zero.  See methods below.
  int meshLock, meshExpandFlag;

  // private helper methods used by FEM interface functions
  void deriveNodes();
  int edgeLocal(elemRef e1, elemRef e2);
  int findEdge(int n1, int n2);
  int addNewEdge(int n1, int n2);
  int getNbrRefOnEdge(int n1, int n2, int *conn, int nGhost, int *gid, 
		      int idx, elemRef *er);
  int hasEdge(int n1, int n2, int *conn, int idx);
  
 public:
  // Data fields for this chunk's array index, and counts of elements,
  // edges, and nodes located on this chunk; numGhosts is numElements
  // plus number of ghost elements surrounding this chunk
  int cid, numElements, numEdges, numNodes, numGhosts, numChunks;

  refineResults *refineResultsStorage;

  // the chunk's components, left public for sanity's sake
  std::vector<element> theElements;
  std::vector<edge> theEdges;
  std::vector<node> theNodes;

  // client to report refinement split information to
  refineClient *theClient;

  // Basic constructors
  chunk(chunkMsg *);
  chunk(CkMigrateMessage *m) : TCharmClient1D(m) { };
  
  void sanityCheck(void);
 
  // entry methods
  void addNode(nodeMsg *);
  void addEdge(edgeMsg *);
  void addElement(elementMsg *);

  void freshen();
  void deriveBorderNodes();
  void tweakMesh();
  void improveChunk();
  void improve();
  
  // deriveEdges creates nodes from the element & ghost info, then creates
  // unique edges for each adjacent pair of nodes
  void deriveEdges(int *conn, int *gid);

  // This initiates a refinement for a single element
  void refineElement(refineMsg *);
  // This loops through all elements performing refinements as needed
  void refiningElements();

  // This initiates a coarsening for a single element
  void coarsenElement(coarsenMsg *);
  // This loops through all elements performing coarsenings as needed
  void coarseningElements();

  // The following methods simply provide remote access to local data
  // See above for details of each
  nodeMsg *getNode(intMsg *);
  refMsg *getEdge(collapseMsg *);
  void setBorder(intMsg *);
  intMsg *safeToMoveNode(nodeMsg *);
  splitOutMsg *split(splitInMsg *);
  void collapseHelp(collapseMsg *);
  void checkPending(refMsg *);
  void checkPending(drefMsg *);
  void updateNode(updateMsg *);
  void updateElement(updateMsg *);
  void updateElementEdge(updateMsg *);
  void updateReferences(updateMsg *);
  doubleMsg *getArea(intMsg *);
  nodeMsg *midpoint(intMsg *);
  intMsg *setPending(intMsg *);
  void unsetPending(intMsg *);
  intMsg *isPending(intMsg *);
  intMsg *lockNode(intMsg *);
  void unlockNode(intMsg *);
  refMsg *getNeighbor(refMsg *);
  refMsg *getNotNode(refMsg *);
  refMsg *getNotElem(refMsg *);
  intMsg *isLongestEdge(refMsg *);
  void setTargetArea(doubleMsg *);
  void resetTargetArea(doubleMsg *);
  void updateEdges(edgeUpdateMsg *);
  void updateNodeCoords(nodeMsg *);
  void reportPos(nodeMsg *);

  // meshLock methods
  void accessLock();  // waits until meshExpandFlag not set, then decs meshLock
  void releaseLock(); // incs meshLock
  void adjustFlag();  // sets meshExpandFlag
  void adjustLock();  // waits until meshLock is 0, then sets it to 1
  void adjustRelease();  // resets meshLock and meshExpandFlag to 0

  // used to print snapshots of all chunks at once (more or less)
  void print();

  // *** These methods are part of the interface with the FEM framework ***
  // create a chunk's mesh data
  void newMesh(int nEl, int nGhost,const int *conn_,const int *gid_, 
	       int idxOffset);
  // multipleRefine sets target areas specified by desiredArea, starts refining
  void multipleRefine(double *desiredArea, refineClient *client);
  // updateNodeCoords sets node coordinates, recalculates element areas
  void updateNodeCoords(int nNode, double *coord, int nEl);


  // add an edge on a remote chunk
  void addRemoteEdge(remoteEdgeMsg *);

  // local methods
  // These access and set local flags
  void setModified() { modified = 1; }
  int isModified() { return modified; }
  void setRefining() { refineInProgress = 1; }
  int isRefining() { return refineInProgress; }

  // these methods allow for run-time additions/modifications to the chunk
  void allocMesh(int nEl);
  void adjustMesh();
  nodeRef *addNode(node& n);
  edgeRef *addEdge(nodeRef& nr1, nodeRef& nr2);
  elemRef *addElement(nodeRef& nr1, nodeRef& nr2, nodeRef& nr3);
  elemRef *addElement(nodeRef& nr1, nodeRef& nr2, nodeRef& nr3,
		      edgeRef& er1, edgeRef& er2, edgeRef& er3);
  void removeNode(intMsg *);
  void removeEdge(intMsg *);
  void removeElement(intMsg *);
  // prints a snapshot of the chunk to file
  void debug_print(int c);
  void out_print();
};

#endif
