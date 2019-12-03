#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <vector>

#include "types.h"

void printEdge(const Edge& e);
void printEdgeList(const EdgeList& el);
void printEdgeQueue(EdgeQueue q);
void printEdgeQueueArray(EdgeQueue* q, int num_partitions);
void printEdgeBatchQueue(EdgeBatchQueue q);
void printEdgeBatchQueueArray(EdgeBatchQueue* q, int num_partitions);
void printMap(const MapTable& mapper);
void printVector(std::vector<float> const& vec);
void printVecOfNodes(std::vector<Node> const& vec);
void printVecOfNodes(std::vector<NodeWeight> const& vec);
void printVecOfVecOfNodes(std::vector<std::vector<Node>> const& v);
void printVecOfVecOfNodes(std::vector<std::vector<NodeWeight>> const& v);

#endif //FUNCTIONS_H_
