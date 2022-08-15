#ifndef ADLISTPERCHUNK_H_
#define ADLISTPERCHUNK_H_

#include <iostream>

#include "abstract_data_struc.h"
#include "print.h"

template <typename U> class adListChunked;

// T can be either node or nodeweight
template <typename T>
class adListPerChunk: public dataStruc {
      friend class adListChunked<T>;  
    private:                
      int64_t num_nodes_initialize;  
      void updateForExistingVertex(const Edge& e);   
      
    public:  
      std::vector<std::vector<T>> neighbors;
      adListPerChunk(bool w, bool d, int64_t _num_nodes);    
      void update(const EdgeList& el) override;
      void print() override;
      int64_t degree(NodeID n);

      int64_t in_degree(NodeID n) { return -1*n; }
      int64_t out_degree(NodeID n) { return -1*n; }

};

template <typename T>
adListPerChunk<T>::adListPerChunk(bool w, bool d, int64_t _num_nodes)
    : dataStruc(w, d), num_nodes_initialize(_num_nodes){ 
        neighbors.resize(num_nodes_initialize);
    }    

template <typename T>
void adListPerChunk<T>::updateForExistingVertex(const Edge& e) {
    NodeID index = e.source;
    NodeID dest = e.destination; 
    int64_t foundEmptySlot = -1;
    for (unsigned int i = 0; i < neighbors[index].size(); i++) {
        NodeID temp = neighbors[index][i].getNodeID();
        if (temp  == dest) {
            neighbors[index][i].setInfo(dest, e.weight);              
            return;
        } 

        // Mark a deleted one.
        if (temp == -1 && foundEmptySlot == -1) {
            foundEmptySlot = i;
        }
    }

    // Found a deleted one.
    if (foundEmptySlot != -1) {
        neighbors[index][foundEmptySlot].setInfo(dest,e.weight);
        return;
    }

    // Otherwise, push back.
    T neighbor;
    neighbor.setInfo(dest, e.weight);           
    neighbors[index].push_back(neighbor);
    return;           
}

template <typename T>
void adListPerChunk<T>::update(const EdgeList& el)
{
    for(auto it=el.begin(); it!=el.end(); it++)
        updateForExistingVertex(*it);      
}

template <typename T>
int64_t adListPerChunk<T>::degree(NodeID n) {
	return neighbors[n].size();
}

template <typename T>
void adListPerChunk<T>::print()
{
    std::cout << " numNodes: " << num_nodes << 
            " numEdges: " << num_edges << 
            " weighted: " << weighted << 
            " directed: " << directed << 
	std::endl;

    /*cout << "Property: "; printVector(property);    
    cout << "out_neighbors: " << endl; printVecOfVecOfNodes(out_neighbors); 
    cout << "in_neighbors: " << endl; printVecOfVecOfNodes(in_neighbors);*/
}
#endif  // ADLISTPERCHUNK_H_
