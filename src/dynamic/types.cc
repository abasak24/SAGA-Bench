#ifndef TYPES_CC
#define TYPES_CC

#include "types.h"

std::ostream& operator<<(std::ostream &out, EdgeID const &id) {
    out << "(" << id.first << "," << id.second << ")";
    return out;
}

std::ostream& operator<<(std::ostream &out, Node const &nd)
{
    out << "(id=" << nd.getNodeID() << ")";
    return out;
}

std::ostream& operator<<(std::ostream &out, NodeWeight const &nw)
{
    out << "(id=" << nw.getNodeID() << ",weight=" << nw.getWeight() << ")";
    return out;
}

#endif
