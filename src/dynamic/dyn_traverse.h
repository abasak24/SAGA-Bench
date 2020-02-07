#ifndef DYN_TRAVERSE_H_
#define DYN_TRAVERSE_H_

#include "types.h"
#include "traversal.h"
#include "../common/timer.h"
#include <iostream>
#include <type_traits>

/* Algorithm: Micro-kernel for serial (single-threaded) neighbor traversal */

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
    Timer t;
    t.Start();

    traverseInNeighbors(ds);        
    if(ds->directed)
	traverseOutNeighbors(ds);  

    t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();         
}
#endif  // DYN_TRAVERSE_H_