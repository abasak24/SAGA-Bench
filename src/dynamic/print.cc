#include "print.h"

void printEdge(const Edge& e)
{
    std::cout << "source:" << e.source
	      << " dest:" << e.destination
	      << " weight:" << e.weight
	      << " batch:" << e.batch_id
	      << " sourceExists:" << e.sourceExists
	      << " destExists:" << e.destExists
	      << std::endl;
}

void printEdgeList(const EdgeList& el)
{
    std::cout << "EdgeList: " << std::endl;
    for (auto it = el.begin(); it != el.end(); it++) {
        printEdge(*it);        
    }
    std::cout << std::endl;
}

// note: there is no efficient way to iterate a queue. So I make a copy 
// in the argument and keep on popping 
void printEdgeQueue(EdgeQueue q)
{
    while (!q.empty()) {
        printEdge(q.front());        
        q.pop();
    }    
}

void printEdgeQueueArray(EdgeQueue* q, int num_partitions)
{
    for (int i=0; i < num_partitions; ++i) {
        printEdgeQueue(q[i]);
	std::cout << std::endl;
    }
}

void printEdgeBatchQueue(EdgeBatchQueue q)
{
    while (!q.empty()) {
        printEdgeList(q.front());                  
        q.pop();
    } 
}

void printEdgeBatchQueueArray(EdgeBatchQueue* q, int num_partitions)
{
    for (int i = 0; i < num_partitions; ++i) {
	std::cout << "Queue " << i << ":" << std::endl;
        printEdgeBatchQueue(q[i]);
	std::cout << std::endl;
    }
}

void printMap(const MapTable& mapper)
{    
    if (!mapper.empty()) {
        for(MapTable::const_iterator it = mapper.begin(); it != mapper.end(); ++it){
	    std::cout << it->first << " => " << it->second << std::endl;
        }
    }    
}

void printVector(std::vector<float> const& vec)
{
    if (vec.empty()) {
	std::cout << std::endl;
    } else {
        for (auto it = vec.begin(); it != vec.end(); it++) {
	    std::cout << *it << " ";
	}
	std::cout << std::endl;
    }
}

void printVecOfNodes(std::vector<Node> const& vec)
{
    if (vec.empty()) {
	std::cout << std::endl;
    } else {
        for (auto it = vec.begin(); it != vec.end(); it++) {
            (*it).printNode();
        }
	std::cout << std::endl;
    }
}

void printVecOfNodes(std::vector<NodeWeight> const& vec)
{
    if (vec.empty()) {
	std::cout << std::endl;
    } else {
        for (auto it = vec.begin(); it != vec.end(); it++) {
            (*it).printNode();
        }
	std::cout << std::endl;
    }
}

void printVecOfVecOfNodes(std::vector<std::vector<Node>> const& v)
{
    if (v.empty()) {
	std::cout << std::endl;
    } else {
        for (auto it = v.begin(); it != v.end(); it++) {
            printVecOfNodes(*it);
        }
	std::cout << std::endl;
    }
}

void printVecOfVecOfNodes(std::vector<std::vector<NodeWeight>> const& v)
{
    if (v.empty()) {
	std::cout << std::endl;
    } else {
        for(auto it = v.begin(); it != v.end(); it++){
            printVecOfNodes(*it);
        }
	std::cout << std::endl;
    }
}
