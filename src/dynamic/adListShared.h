#ifndef ADLISTSHARED_H_
#define ADLISTSHARED_H_

#include <iostream>
#include <thread>
#include <stdlib.h>
#include <mutex>

#include <cassert>
// #include<memory>
#include "x86_full_empty.h"
#include "stinger_atomics.h"

#include "abstract_data_struc.h"
#include "print.h"

bool compare_and_swap(bool &x, const bool &old_val, const bool &new_val);

// T can be either node or nodeweight
template <typename T>
class adListShared: public dataStruc {
    private:  
      void updateExistingEdge(NodeID self, unsigned int index, T new_neighbor, bool in_neighbor);    
      void search_and_insert_edge(const Edge& e, bool source, std::vector<T> &neighborList, bool in_neighbor);          
      void updateForExistingVertex(const Edge& e, bool source);   
      void processMetaData(const Edge& e, bool source);     

      std::vector<std::unique_ptr<std::mutex>> in_mutex, out_mutex;
      int64_t num_nodes_initialize;
      
    public:  
      std::vector<std::vector<T>> out_neighbors;
      std::vector<std::vector<T>> in_neighbors;  
      adListShared(bool w, bool d, int64_t _num_nodes);    
      void update(const EdgeList& el) override;
      void print() override;
      int64_t in_degree(NodeID n) override;
      int64_t out_degree(NodeID n) override;      
};

template <typename T>
adListShared<T>::adListShared(bool w, bool d, int64_t _num_nodes): dataStruc(w, d), num_nodes_initialize(_num_nodes){    

    // initialize 1) property 2) affected 3) vertices vectors 4) mutex
    property.resize(num_nodes_initialize, -1);    
    affected.resize(num_nodes_initialize); affected.fill(false);
   
    out_neighbors.resize(num_nodes_initialize);    
    in_neighbors.resize(num_nodes_initialize);
    
    // Malloc for mutex.
    out_mutex.resize(num_nodes_initialize);
    in_mutex.resize(num_nodes_initialize);
    for  (unsigned int k = 0; k< num_nodes_initialize; k++){
        out_mutex[k].reset(new std::mutex());
        in_mutex[k].reset(new std::mutex());
    }
    
}    

template <typename T>
void adListShared<T>::processMetaData(const Edge& e, bool source) {
    bool exists;
    if(source) exists = e.sourceExists;
    else exists = e.destExists;

    // using CAS operations implemented in GAP 
    if(source){
        NodeID v = e.source;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    else{
        NodeID v = e.destination;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    
    if(exists){       
        stinger_int64_fetch_add(&num_edges, 1);                 
    }
    else{
        stinger_int64_fetch_add(&num_nodes, 1); 
        stinger_int64_fetch_add(&num_edges, 1);            
    }  
}

template <typename T>
void adListShared<T>::updateForExistingVertex(const Edge& e, bool source) {
    NodeID index;
    if(source)
	    index = e.source;
    else
	    index = e.destination;
    if (source || (!source && !directed)) {
        NodeID dest;
        if(source)
	        dest = e.destination;
        else
	        dest = e.source;
        //search for the edge first in out_neighbors 
        int64_t foundEmptySlot = -1;

        // guard the mutex
        std::lock_guard<std::mutex> guard(*out_mutex[index]);
        NodeID temp;
        for (unsigned int i = 0; i < out_neighbors[index].size(); i++) {
            temp = out_neighbors[index][i].getNodeID();
            if (temp  == dest) {
                out_neighbors[index][i].setInfo(dest, e.weight);           
                return;
            }

            // Mark a deleted one.
            if (temp == -1 && foundEmptySlot == -1) 
                foundEmptySlot = i;
        }

        if (foundEmptySlot == -1) {
            T neighbor;
            neighbor.setInfo(dest, e.weight);           
            out_neighbors[index].push_back(neighbor);
            return; 
        }

        // Go into the slot
        out_neighbors[index][foundEmptySlot].setInfo(dest, e.weight);
        return;
    } 
    else if (!source && directed) {
        // in_neighbors    
        // search for the edge first in in_neighbors 
        int64_t foundEmptySlot = -1;
        NodeID temp;
        
        // guard the mutex
        std::lock_guard<std::mutex> guard(*in_mutex[index]);
        for(unsigned int i = 0; i < in_neighbors[index].size(); i++){
            temp = in_neighbors[index][i].getNodeID();
            if(temp == e.source){
                in_neighbors[index][i].setInfo(e.source, e.weight);             
                return;
            }  

            // Mark a deleted one.
            if (temp == -1 && foundEmptySlot == -1) 
                foundEmptySlot = i;          
        }

        if (foundEmptySlot == -1) {
            T neighbor;
            neighbor.setInfo(e.source, e.weight);           
            in_neighbors[index].push_back(neighbor);
            return; 
        }

        // Go into the slot
        in_neighbors[index][foundEmptySlot].setInfo(e.source, e.weight);
        return;          
    }
}

template <typename T>
void adListShared<T>::update(const EdgeList& el)
{
    # pragma omp parallel for 
    // for(auto it=el.begin(); it!=el.end(); it++){
    for (unsigned int k = 0; k < el.size(); k ++) {
        // examine source vertex
        //bool exists = vertexExists(*it, true); 
        //if(!exists) updateForNewVertex(*it, true);
        processMetaData(el[k], true);
        updateForExistingVertex(el[k], true);
    
        // examine destination vertex 
        //bool exists1 = vertexExists(*it, false); 
        //if(!exists1) updateForNewVertex(*it, false);
        processMetaData(el[k], false);
        updateForExistingVertex(el[k], false); 
    }               
}

template <typename T>
int64_t adListShared<T>::in_degree(NodeID n)
{
    if(directed) {
        std::lock_guard<std::mutex> guard(*in_mutex[n]);
	    return in_neighbors[n].size(); }
    else {
        std::lock_guard<std::mutex> guard(*out_mutex[n]);
	    return out_neighbors[n].size(); }
}

template <typename T>
int64_t adListShared<T>::out_degree(NodeID n)
{
    std::lock_guard<std::mutex> guard(*out_mutex[n]);
    return out_neighbors[n].size();    
}

template <typename T>
void adListShared<T>::print()
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
#endif  // ADLISTSHARED_H_


// #ifndef ADLIST_H_
// #define ADLIST_H_

// #include <iostream>
// #include <iomanip>
// #include <stdlib.h>

// #include "abstract_data_struc.h"
// #include "print.h"

// #include <cassert>
// // #include<memory>
// #include "x86_full_empty.h"
// #include "stinger_atomics.h"

// using namespace std;

// bool compare_and_swap(bool &x, const bool &old_val, const bool &new_val);
// //class adListPart<T>;

// // T can be either node or nodeweight
// template <typename T>
// class adList: public dataStruc {
// //      friend class adListPart<T>;
//     private:
//       int64_t num_nodes_initialize; // The max amount of nodes we would initialize.
      
//       void processMetaData(const Edge& e, bool source);
//       void updateForVertex(const Edge& e, bool source);    
      
//       void updateExistingEdge(NodeID self, unsigned int index, T new_neighbor, bool in_neighbor);    
//       void search_and_insert_edge(const Edge& e, bool source, std::vector<T> &neighborList, bool in_neighbor); 

//     //   vector<bool> in_markers;
//     //   vector<bool> out_markers;

//       bool* in_markers;
//       bool* out_markers;
      
//     public:  

//       std::vector<std::vector<T>> out_neighbors;
//       std::vector<std::vector<T>> in_neighbors;
//     //   vector<T>* out_neighbors, in_neighbors;
//       adList(bool w, bool d, int64_t _num_nodes);
//       adList(bool w, bool d);
//       ~adList();   
//       void update(const EdgeList& el) override;
//       void print() override;
//       int64_t in_degree(NodeID n) override;
//       int64_t out_degree(NodeID n) override;      
// };

// template <typename T>
// adList<T>::adList(bool w, bool d, int64_t _num_nodes)
// : dataStruc(w, d){
//     num_nodes_initialize = _num_nodes;

//     // initialize 1) property 2) affected 3) vertices vectors 4) markers
//     property.resize(num_nodes_initialize, -1);
//     affected.resize(num_nodes_initialize); affected.fill(false);

//     out_neighbors.resize(num_nodes_initialize);
//     in_neighbors.resize(num_nodes_initialize);

//     // out_neighbors = new vector<T>[num_nodes_initialize];
//     // in_neighbors = new vector<T>[num_nodes_initialize];

//     in_markers = new bool[num_nodes_initialize];
//     out_markers = new bool[num_nodes_initialize];
//     //in_markers.resize(num_nodes_initialize);
//     //out_markers.resize(num_nodes_initialize);
//     for (unsigned int k = 0; k < num_nodes_initialize; k++){
//         in_markers[k] = 1;
//         out_markers[k] = 1;
//     }
// }    

// template <typename T>
// adList<T>::~adList(){
//    delete []in_markers;
//    delete []out_markers;
// //    delete []out_neighbors;
// //    delete []in_neighbors;
// }

// template <typename T>
// void adList<T>::processMetaData(const Edge& e, bool source)
// {
//     bool exists;
//     if(source) exists = e.sourceExists;
//     else exists = e.destExists;

//     // using CAS operations implemented in GAP 
//     if(source){
//         NodeID v = e.source;
//         bool aff = affected[v];
//         if(!aff){
//             compare_and_swap(affected[v], aff, true);
//         }
//     }
//     else{
//         NodeID v = e.destination;
//         bool aff = affected[v];
//         if(!aff){
//             compare_and_swap(affected[v], aff, true);
//         }
//     }
    
//     if(exists){       
//         stinger_int64_fetch_add(&num_edges, 1);                 
//     }

//     else{
//         stinger_int64_fetch_add(&num_nodes, 1); 
//         stinger_int64_fetch_add(&num_edges, 1);            
//     }  
// }

// template <typename T>
// void adList<T>::updateExistingEdge(NodeID self, unsigned int index, T new_neighbor, bool in_neighbor){
//     if (weighted){
//         if (in_neighbor)
//             in_neighbors[self][index].setInfo(new_neighbor.getNodeID(), new_neighbor.getWeight());
//         else
//             out_neighbors[self][index].setInfo(new_neighbor.getNodeID(), new_neighbor.getWeight());
//     }
// }

// template <typename T>
// void adList<T>::search_and_insert_edge(const Edge& e, bool source, std::vector<T> &neighborList, bool in_neighbor){
//     //create a new T 
//     T neighbor;
//     NodeID selfID;
//     if (source) { 
// 	    neighbor.setInfo(e.destination, e.weight);
//         selfID = e.source;
//     } 
//     else {
// 	    neighbor.setInfo(e.source, e.weight);
//         selfID = e.destination;
//     }

//     // 1. search whether the edge exists
//     bool* markerPtr; // pointer to the marker
//     if (selfID >= num_nodes_initialize) cout << "wrong ID" <<endl;
//     if (in_neighbor){
//         markerPtr = in_markers + selfID;
//     }
//     else{
//         markerPtr = out_markers + selfID;
//     }
//     // start from the beginning
//     bool _adr = readfe_bool(markerPtr);
//     unsigned int len = neighborList.size();
//     // writexf_bool(markerPtr, _adr);
//     for ( unsigned int k = 0; k < len; k++) {
//         if (neighborList[k].getNodeID() == neighbor.getNodeID()) {
//             // found the existing edge
//             // bool _adr = readfe_bool(markerPtr);              
//             updateExistingEdge(selfID, k, neighbor, in_neighbor);
//             writexf_bool(markerPtr, _adr);
//             return ;
//         } 
//     }

//     // 2. Not found, try to insert
//     // _adr = readfe_bool(markerPtr);
//     // for (unsigned int k = 0; k < neighborList.size(); k++ ){
//     //     if (neighborList[k].getNodeID() == neighbor.getNodeID()) {
//     //         updateExistingEdge(selfID, k, neighbor, in_neighbor);
//     //         writexf_bool(markerPtr, _adr);
//     //         return;
//     //     }
//     //     else if (neighborList[k].getNodeID() == -1) {
//     //         updateExistingEdge(selfID, k, neighbor, in_neighbor);
//     //         writexf_bool(markerPtr, _adr);
//     //         return;
//     //     }
//     // }
//     neighborList.push_back(neighbor);
//     writexf_bool(markerPtr, _adr);
//     return;
// }

// template <typename T>
// void adList<T>::updateForVertex(const Edge& e, bool source){
//     NodeID index;
//     std::vector<T> neighbors;
//     if(source || !directed) {
// 	    index = e.source;
//         //cout << index << endl;
//         neighbors = out_neighbors[index];
//     }
//     else {
// 	    index = e.destination;
//         neighbors = in_neighbors[index];
//     }

//     if(source || !directed){       
//         search_and_insert_edge(e, source, out_neighbors[index], false);             
//     }
//     else if(!source && directed){        
//         search_and_insert_edge(e, source, in_neighbors[index], true);             
//     } 
// }


// template <typename T>
// void adList<T>::update(const EdgeList& el)
// {
//     cout<<"begin update"<<endl;
//     # pragma omp parallel for
//     for(unsigned int i=0; i<el.size(); i++){
//         // examine source vertex
//         processMetaData(el[i], true);
//         updateForVertex(el[i], true);
        
//         // examine destination vertex 
//         processMetaData(el[i], false);
//         updateForVertex(el[i], false);
//     }      
//     cout<<"end update"<<endl;
// }

// template <typename T>
// int64_t adList<T>::in_degree(NodeID n)
// {
//     NodeID id = n;
//     if(directed) {
//         bool* markerPtr;
//         markerPtr = &(in_markers[id]);
//         bool _adr = readfe_bool(markerPtr);
//         int64_t size = in_neighbors[id].size();
//         writexf_bool(markerPtr, _adr);
//         return size;
//     }
//     else {
//         bool* markerPtr;
//         markerPtr = &out_markers[id];
//         bool _adr = readfe_bool(markerPtr);
//         int64_t size = out_neighbors[id].size();
//         writexf_bool(markerPtr, _adr);
//         return size;

//     }
// }

// template <typename T>
// int64_t adList<T>::out_degree(NodeID n)
// {   
//     NodeID id = n;
//     bool* markerPtr;
//     markerPtr = &out_markers[id];
//     bool _adr = readfe_bool(markerPtr);     
//     int64_t size = out_neighbors[id].size();
//     writexf_bool(markerPtr, _adr);
//     return size;  
// }


// template <typename T>
// void adList<T>::print()
// {
//     std::cout << " numNodes: " << num_nodes << 
//             " numEdges: " << num_edges << 
//             " weighted: " << weighted << 
//             " directed: " << directed << 
// 	std::endl;

//     /*cout << "Property: "; printVector(property);    
//     cout << "out_neighbors: " << endl; printVecOfVecOfNodes(out_neighbors); 
//     cout << "in_neighbors: " << endl; printVecOfVecOfNodes(in_neighbors);*/
// }
// #endif  // ADLIST_H_

