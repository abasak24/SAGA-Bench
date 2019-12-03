#ifndef DYN_TRAVERSE_H_
#define DYN_TRAVERSE_H_

#include "types.h"
#include "traversal.h"

#include <iostream>
#include <type_traits>

/*
Algorithm: Micro-kernel for serial neighbor traversal 
*/

template<typename T>
void traverseInNeighbors(T* ds)
{               
    for (NodeID n = 0; n < ds->num_nodes; n++) {
	std::cout << "In-Neighbors of Node " << n << ":  ";
	// should be for (auto it: in_neigh(n, ds));
        neighborhood<T> neigh = in_neigh(n, ds);    
        for (neighborhood_iter<T> it = neigh.begin(), end = neigh.end(); it != end; ++it)
	{
	    std::cout << "(" << *it << " , " << it.extractWeight() << ")   ";                   
    }      
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
}

template<typename T>
void traverseOutNeighbors(T* ds)
{
    for (NodeID n = 0; n < ds->num_nodes; n++) {
        std::cout << "Out-Neighbors of Node " << n << ":  ";  
        neighborhood<T> neigh = out_neigh(n, ds);
        for (neighborhood_iter<T> it = neigh.begin(), end = neigh.end();
	     it != end;
	     ++it)
	{
            std::cout << "(" << *it << " , " << it.extractWeight() << ")   ";                   
        }      
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
}

template<typename T>
void traverseAlg(T* ds)
{
    traverseInNeighbors(ds);        
    if(ds->directed)
	traverseOutNeighbors(ds);           
}
#endif  // DYN_TRAVERSE_H_