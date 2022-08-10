#ifndef STINGER_H_
#define STINGER_H_

#include "abstract_data_struc.h"
#include "x86_full_empty.h"
#include "print.h"
#include <cassert>

using namespace std;

const int64_t NUM_EDGES_PER_BLOCK = 16; 

bool compare_and_swap(bool &x, const bool &old_val, const bool &new_val);

struct stinger_edge{
    NodeID neighbor;
    Weight weight;
    int64_t timeFirst; /**< Random placeholder for locking empty space */

    stinger_edge():neighbor(-1), weight(-1), timeFirst(0){}

    stinger_edge(const Edge& e, bool source)
    :timeFirst(0)
    {
        if(source) neighbor = e.destination;
        else neighbor = e.source;
        weight = e.weight;
    }
};

struct stinger_eb
{
    NodeID node;            /**< Source vertex ID associated with this edgeblock*/
    int64_t numEdges;	    /**< Number of valid edges in the block */         
    int64_t high;           /**< High water mark */
    stinger_eb* next;	    /**< Pointer to the next edge block */       
    stinger_edge edges[NUM_EDGES_PER_BLOCK]; /**< Array of edges, Node or NodeWeight*/    
    
    stinger_eb(NodeID n)
    :node(n), numEdges(0), high(-1), next(nullptr){}
};

struct stinger_vertex
{
    NodeID node;
    int64_t in_degree;
    int64_t out_degree;
    stinger_eb* in_neighbors;	  /**< Pointer to the first edge block in in_neighbors */
    stinger_eb* out_neighbors;  /**< Pointer to the first edge block in out_neighbors */

    stinger_vertex(NodeID _n)
    :node(_n),
    in_degree(0),
    out_degree(0),
    in_neighbors(nullptr),
    out_neighbors(nullptr){}                    
};

class stinger: public dataStruc{
    private:                     
      int64_t num_nodes_initialize;    /**< # of nodes we initialize with, not actual # of nodes */
      //bool vertexExists(const Edge& e, bool source);
      //void updateForNewVertex(const Edge& e, bool source);
      //void updateForExistingVertex(const Edge& e, bool source); 
      
      /* function vertexExists does not need to be bool anymore
         It is sufficient to make it a void function for some statistics preprocessing 
      */
      void processMetaData(const Edge& e, bool source); 

      /* it looks like it is OK to just update for all vertices in same way 
         no need to distinguish between new vertex and existing vertex 
      */
      void updateForVertex(const Edge& e, bool source); 

      void print_eb(stinger_eb* eb);
      void in_degree_increment_atomic(NodeID n, int64_t degree);
      void out_degree_increment_atomic(NodeID n, int64_t degree);        
     
      void update_edge_data(stinger_eb* eb, int index, NodeID n, Weight w, bool in_neighbor);    
      void search_and_insert_edge(const Edge& e, bool source, stinger_eb* eb, bool in_neighbor); 
     
    public:          
      vector<stinger_vertex> vertices; // array of stinger_vertex  
      stinger(bool w, bool d, int64_t _num_nodes);    
      void update(const EdgeList& el) override;
      void print() override;
      int64_t in_degree(NodeID n) override;
      int64_t out_degree(NodeID n) override; 
};
#endif // STINGER_H_