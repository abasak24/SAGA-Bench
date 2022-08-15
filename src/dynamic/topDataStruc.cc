#include "topDataStruc.h"

dataStruc* createDataStruc(const std::string& type, bool weighted, bool directed, int64_t num_nodes, int64_t num_threads)
{        
    if (type == "adList") {
      if (weighted)
	    return new adList<NodeWeight>(weighted, directed); 
      else
	    return new adList<Node>(weighted, directed);
    } else if (type == "adListShared") {       
      if (weighted)
	    return new adListShared<NodeWeight>(weighted, directed, num_nodes); 
      else
	    return new adListShared<Node>(weighted, directed, num_nodes);
    } else if (type == "adListChunked") {
        if (weighted)
            return new adListChunked<NodeWeight>(weighted, directed, num_nodes, num_threads); 
        else
            return new adListChunked<Node>(weighted, directed, num_nodes, num_threads);
    } else if (type == "degAwareRHH") {
	if (weighted)
	    return new darhh<NodeWeight>(weighted, directed, num_nodes, num_threads);
	else
	    return new darhh<Node>(weighted, directed, num_nodes, num_threads);
    } else if(type == "stinger") {
        return new stinger(weighted, directed, num_nodes);         
    }else{
        cout << "ERROR! Unrecognized Data Structure Type!" << endl;
    } 
    return new adListShared<NodeWeight>(weighted, directed, num_nodes);    
 }
