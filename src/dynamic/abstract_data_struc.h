#ifndef ABSTRACT_DATA_STRUC_H
#define ABSTRACT_DATA_STRUC_H

#include <vector>
#include <cstdint>

#include "types.h"
#include "../common/pvector.h"

class dataStruc {
public:        
    int64_t num_nodes = 0;
    int64_t num_edges = 0; 
    bool weighted;
    bool directed;
    std::vector<float> property;
    pvector<bool> affected;
    dataStruc(bool _weighted, bool _directed):
        weighted(_weighted),
        directed(_directed) {}

    virtual void update(const EdgeList& el) = 0;
    virtual void print() = 0;
    virtual ~dataStruc(){}
    virtual int64_t in_degree(NodeID n) = 0;
    virtual int64_t out_degree(NodeID n) = 0;      
};
#endif