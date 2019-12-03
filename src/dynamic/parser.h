#ifndef PARSER_H
#define PARSER_H

#include <string>

struct cmd_args {
    int batch_size = 0;
    bool directed = false;
    bool weighted = false;
    int64_t num_nodes = 0;
    std::string filename;
    std::string type = "adList";
    std::string algorithm = "traverse";
    int8_t flags = 0;
    int64_t num_threads = 16; // default
};

std::string getSuffix(std::string filename);
bool supportedAlg(const std::string &alg);
bool supportedDataStruc(const std::string &type);
void printUsage();
cmd_args parse(int argc, char *argv[]);

#endif
