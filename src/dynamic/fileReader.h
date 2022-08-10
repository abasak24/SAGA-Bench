#ifndef FILEREADER_H_
#define FILEREADER_H_

#include <sstream>
#include <fstream>

#include "types.h"

using namespace std;

/* 
1) Read edge streams from files
2) Maintain MapTable
3) Assign logical IDs
4) Assign batch IDs 

TO DO: Add support for other file types
*/

Edge convertCSVLineIntoEdge(const char delim, const string& line, bool weighted){
    // extract the numbers from the line-> a,b,c,d
    stringstream ss(line); // put the line in an internal stream     
    NodeID source, dest; /*int64_t time;*/
    string data;
    
    getline(ss, data, delim); source = stol(data); 
    getline(ss, data, delim); dest = stol(data); 
    getline(ss, data, delim); /*time = stol(data);*/

    if(weighted){
        Weight w; 
        getline(ss, data, delim);
        w = stol(data);        
        Edge e(source, dest, w);
        return e;
    }

    Edge e(source, dest);
    return e;    
}

// return true if a mapping is found, otherwise false
bool assignLogicalID(NodeID& n, MapTable& VMap, NodeID& lastAssignedLogicalID){
    if(VMap.empty()){
        // this is the first vertex ever         
        VMap[n] = 0;  
        n = 0;
        lastAssignedLogicalID=0;
        return false;        
    }
    
    MapTable::iterator it;
    it = VMap.find(n);
    if(it != VMap.end()){
        // vertex exists
        n = it->second;
        return true;
    }
    
    else{
        // vertex does not exist   
        lastAssignedLogicalID++;           
        VMap[n] = lastAssignedLogicalID; 
        n = lastAssignedLogicalID;          
        return false;
    }
}


EdgeList readBatchFromCSV(ifstream& in, int batchSize, int batch_id, bool weighted, MapTable& VMap, NodeID& lastAssignedLogicalID){
    EdgeList el;
    int edgecount = 0;
    string line;      

    while(getline(in, line)){
        if(line != ""){          
            Edge e = convertCSVLineIntoEdge(',', line, weighted);
            if(assignLogicalID(e.source, VMap, lastAssignedLogicalID)) e.sourceExists = true;
            if(assignLogicalID(e.destination, VMap, lastAssignedLogicalID)) e.destExists = true;
            e.batch_id = batch_id;
            el.push_back(e);
            edgecount++;  
            if(edgecount == batchSize) break;   
        }             
    }            
    return el;
}
#endif  // FILEREADER_H_
