#ifndef BUILDER_H
#define BUILDER_H

#include <mutex>

#include "types.h"
#include "abstract_data_struc.h"

void* dequeAndInsertEdge(
    std::string datatype,
    dataStruc *ds,
    EdgeBatchQueue *q,
    std::mutex *lock,
    std::string alg,
    bool *loop);

#endif
