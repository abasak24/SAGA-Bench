#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <vector>

/* Basic building blocks for node and its variations, typedefs. */

typedef int64_t NodeID;
typedef int64_t Weight;
typedef int PID;
typedef std::map<NodeID, NodeID> MapTable;
static const int32_t kRandSeed = 27491095;
const float kDistInf = std::numeric_limits<float>::max()/2;
const size_t kMaxBin = std::numeric_limits<size_t>::max()/2;


class EdgeID: public std::pair<NodeID, NodeID> {
public:
    EdgeID(): std::pair<NodeID, NodeID>(0, 0) {}
    EdgeID(NodeID a, NodeID b): std::pair<NodeID, NodeID>(a, b) {}
    inline int operator %(int mod) const {
	return this->first % mod;
    }
};

class BaseNode {
    public: 
      virtual NodeID getNodeID() const = 0;  
      virtual Weight getWeight() const = 0;  
      virtual void setInfo(NodeID n, Weight w) = 0;
      virtual void printNode() const = 0;      
};

class Node: public BaseNode {
private:
    NodeID node;
public:
    Node(): node(-1){}
    Node(NodeID n):node(n){}
    void setInfo(NodeID n, Weight w){
	(void)w;
	node = n;
    }
    NodeID getNodeID() const {return node;}
    Weight getWeight() const {return -1;}
    void printNode() const{
	std::cout << node << "  ";
    }
}; 

class NodeWeight: public BaseNode {
    NodeID node;
    Weight weight;
    public: 
       NodeWeight():node(-1), weight(-1){}
       NodeWeight(NodeID n):node(n), weight(-1){}
       NodeWeight(NodeID n, Weight w): node(n), weight(w) {}
       void setInfo(NodeID n, Weight w){
           node = n; weight = w;
       }
       Weight getWeight() const {return weight;}
       NodeID getNodeID() const {return node;}
       void printNode() const{
	   std::cout << "(" << node << "," << weight  << ")" << "  ";
       }
       bool operator< (const NodeWeight& rhs) const {
           return node == rhs.getNodeID() ? weight < rhs.getWeight() : node < rhs.getNodeID();
       }
       bool operator== (const NodeWeight& rhs) const {
           return (node == rhs.getNodeID()) && (weight == rhs.getWeight());
       }   
};

struct Edge {
    NodeID source;
    NodeID destination;
    Weight weight;    
    int batch_id;
    bool sourceExists;
    bool destExists;
    Edge(NodeID s, NodeID d, Weight w, bool se, bool de):
	source(s),
	destination(d),
	weight(w),
	batch_id(-1),
	sourceExists(se),
	destExists(de) {}
    Edge(NodeID s, NodeID d, bool se, bool de): Edge(s, d, -1, se, de) {}
    Edge(NodeID s, NodeID d, Weight w): Edge(s, d, w, false, false) {}
    Edge(NodeID s, NodeID d): Edge(s, d, -1) {}
    Edge(){}
    Edge reverse() const {
	return Edge(destination, source, weight, destExists, sourceExists);
    }
};

typedef std::vector<Edge> EdgeList;
typedef std::queue<Edge> EdgeQueue;
typedef std::queue<EdgeList> EdgeBatchQueue;

std::ostream& operator<<(std::ostream &out, EdgeID const &id);
std::ostream& operator<<(std::ostream &out, Node const &nd);
std::ostream& operator<<(std::ostream &out, NodeWeight const &nw);

#endif  // TYPES_H_
