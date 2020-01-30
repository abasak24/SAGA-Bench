#ifndef SOURCE_PICKER_DYNAMIC_H_
#define SOURCE_PICKER_DYNAMIC_H_

#include "abstract_data_struc.h"
#include "types.h"
#include <random>

/* This implementation has been borrowed from GAP Benchmark Suite (https://github.com/sbeamer/gapbs) 
   and modified for SAGA-Bench. */

// Used to pick random non-zero degree starting points for search algorithms
class DynamicSourcePicker {
 public:
  explicit DynamicSourcePicker(dataStruc* p, NodeID given_source = -1)
      : ds(p),            
        given_source(given_source), 
        rng(kRandSeed), 
        udist(0, (ds->num_nodes)-1)
        {}

  NodeID PickNext() {
    if (given_source != -1)
      return given_source;

    if(ds->num_nodes == 0) return -1;
    
    NodeID source;
    int32_t num_trials = 0;
    do {
      num_trials++;
      source = udist(rng);      
    } while ((ds->out_degree(source) == 0) && (num_trials < ds->num_nodes));
    
    if(num_trials == ds->num_nodes) source = -1;
    return source;
  }

 private:
  dataStruc* ds;
  NodeID given_source;
  std::mt19937 rng;
  std::uniform_int_distribution<NodeID> udist;  
};
#endif  // SOURCE_PICKER_DYNAMIC_H 
