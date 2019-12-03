#ifndef TOPDATASTRUC_H_
#define TOPDATASTRUC_H_

#include <string>

#include "adList.h"
#include "adListShared.h"
#include "stinger.h"
#include "darhh.h"
#include "adListChunked.h"

dataStruc* createDataStruc(const std::string& type, bool weighted, bool directed, int64_t num_nodes, int64_t num_threads);
#endif