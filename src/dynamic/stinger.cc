#include "stinger.h"

bool compare_and_swap(bool &x, const bool &old_val, const bool &new_val){
    return __sync_bool_compare_and_swap(&x, old_val, new_val);
}

stinger::stinger(bool w, bool d, int64_t _num_nodes):
dataStruc(w,d){        
    //cout << "Created Stinger" << endl;
    num_nodes_initialize = _num_nodes;

    // initialize 1) property 2) affected 3) vertices vectors 
    property.resize(num_nodes_initialize, -1);
    affected.resize(num_nodes_initialize); affected.fill(false);
    
    // we also create the first edgeblock for in-neighbor and out-neighbor 
    for(NodeID i=0; i<num_nodes_initialize; i++){
        stinger_vertex v(i);                
        v.out_neighbors = new stinger_eb(i);
        if(directed) v.in_neighbors = new stinger_eb(i);
        vertices.push_back(v); 
    }
}

void stinger::in_degree_increment_atomic(NodeID n, int64_t degree){
    stinger_int64_fetch_add(&(vertices[n].in_degree), degree);
}

void stinger::out_degree_increment_atomic(NodeID n, int64_t degree){
    stinger_int64_fetch_add(&(vertices[n].out_degree), degree);
}

// NodeID n = it is the neighbor ID 
void stinger::update_edge_data(stinger_eb* eb, int index, NodeID n, Weight w, bool in_neighbor){
    stinger_edge* e = eb->edges + index;
    
    Weight weight = readfe(&(e->weight)); //acquire lock on weight
    weight = w;

    // if this is new edge     
    if((e->neighbor < 0) || (index >= eb->high)){
        // register edge         
        e->neighbor = n;
        stinger_int64_fetch_add(&eb->numEdges, 1);

        if(in_neighbor) in_degree_increment_atomic(eb->node, 1);
        else out_degree_increment_atomic(eb->node, 1);

        if(index >= eb->high) 
             eb->high = index + 1;
    }
    writeef(&(e->weight), weight); // unlock weight 
}

void stinger::processMetaData(const Edge& e, bool source)
{
    bool exists;
    if(source) exists = e.sourceExists;
    else exists = e.destExists;

    // using CAS operations implemented in GAP 
    if(source){
        NodeID v = e.source;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    else{
        NodeID v = e.destination;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    
    if(exists){       
        stinger_int64_fetch_add(&num_edges, 1);                 
    }

    else{
        stinger_int64_fetch_add(&num_nodes, 1); 
        stinger_int64_fetch_add(&num_edges, 1);            
    }  
}

void stinger::search_and_insert_edge(const Edge& e, bool source, stinger_eb* eb, bool in_neighbor)
{
    //make stinger edge 
    stinger_edge edge(e, source);
    NodeID dest = edge.neighbor;
    Weight weight = edge.weight;    

    // check if edge already exists    
    for(stinger_eb* tmp = eb; tmp != nullptr; tmp = (stinger_eb*)readff((int64_t *)&tmp->next)){
        int64_t k, endk;
        endk = tmp->high; 
        for(k=0; k < endk; k++){            
            if (dest == (tmp->edges[k].neighbor)){
                // found edge; update weight 
                update_edge_data(tmp, k, dest, weight, in_neighbor);
                return;
            }
        }
    }

    stinger_eb** cur_loc = &eb; // pointer to pointer to manipulate stinger_ebs
    stinger_eb* tmp; // variable in for loop
    stinger_eb* cur_eb; // reset which eb to start from in each search pass 

    // Did not find edge; second pass 
    while(1){        
        cur_eb = (stinger_eb*)readff((int64_t *)cur_loc);

        for(tmp = cur_eb; tmp != nullptr; tmp = (stinger_eb*)readff((int64_t *)&tmp->next)){
            int64_t k, endk;
            endk = tmp->high;

            for(k=0; k < NUM_EDGES_PER_BLOCK; k++){
                NodeID myNeighbor = (tmp->edges[k].neighbor);

                if ((dest == myNeighbor) && (k < endk)){
                    // found edge; update weight 
                    update_edge_data(tmp, k, dest, weight, in_neighbor);
                    return;
                }

                if(myNeighbor < 0 || k >= endk){
                    // Found an empty slot for the edge, lock it and check again to make sure                    
                    int64_t timefirst = readfe(&(tmp->edges[k].timeFirst));
                    NodeID thisEdge = (tmp->edges[k].neighbor);
                    endk = tmp->high;

                    if (thisEdge < 0 || k >= endk) {
                        // Slot is empty, add the edge
                        update_edge_data(tmp, k, dest, weight, in_neighbor);
                        writexf(&(tmp->edges[k].timeFirst), timefirst);
                        return;
                    } 
                    
                    else if (dest == thisEdge) {
                        // Another thread just added the edge. Do a normal update                        
                        update_edge_data(tmp, k, dest, weight, in_neighbor);        
                        writexf(&(tmp->edges[k].timeFirst), timefirst);               
                        return;
                    } 
                    
                    else {
                        // Another thread claimed the slot for a different edge, unlock and keep looking
                        writexf(&(tmp->edges[k].timeFirst), timefirst); 
                    }
                }
            }
            cur_loc = &(tmp->next);             
        }

        /* 3: Needs a new block to be inserted at end of list. */
        // Try to lock the tail pointer of the last block        
        stinger_eb* old_eb = (stinger_eb*)readfe((int64_t*)cur_loc);
        if (!old_eb){
            // create new edge block 
            NodeID s;
            if(source) s = e.source; 
            else s = e.destination;

            stinger_eb* newBlock = new stinger_eb(s);
            
            // Add edge to the first slot in the new edge block 
            update_edge_data(newBlock, 0, dest, weight, in_neighbor);
                        
            writeef((int64_t*)cur_loc, *((int64_t*)&newBlock));
            return;
        }
        else{
            // Another thread already added a block, unlock and keep searching
            writeef((int64_t*)cur_loc, *((int64_t*)&old_eb));
        }
    }    
}

void stinger::updateForVertex(const Edge& e, bool source){
    NodeID n;
    if(source) n = e.source;
    else n = e.destination;
    stinger_vertex* node = &vertices[n];

    if(source || (!source && !directed)){     
        assert(node->out_neighbors != nullptr);   
        search_and_insert_edge(e, source, node->out_neighbors, false);             
    }
    else if(!source && directed){ 
        assert(node->in_neighbors != nullptr);        
        search_and_insert_edge(e, source, node->in_neighbors, true);             
    } 
}

void stinger::update(const EdgeList& el)
{
    #pragma omp parallel for 
    for(unsigned int i=0; i<el.size(); i++){
        // examine source vertex 
        processMetaData(el[i], true); 
        updateForVertex(el[i], true);
        
        // examine destination vertex 
        processMetaData(el[i], false);
        updateForVertex(el[i], false);  
    }              
}

int64_t stinger::in_degree(NodeID n)
{
    if(directed) return vertices[n].in_degree;
    else return vertices[n].out_degree;
}

int64_t stinger::out_degree(NodeID n)
{
    return vertices[n].out_degree;    
}

void stinger::print_eb(stinger_eb* eb)
{
    cout << "  NODE:   " << eb->node << endl;
    cout << "  NEXT:   " << eb->next << endl;
    cout << "  NUMEDGS:   " << eb->numEdges << endl;
    cout << "  HIGH:      " << eb->high << endl;
    cout << "  EDGES: " << endl;

    if(!(eb->numEdges == 0)){
        for (uint64_t j = 0; j < NUM_EDGES_PER_BLOCK; j++){
            cout << "   TO: " << eb->edges[j].neighbor << "   WGT: " << eb->edges[j].weight << 
                    "   TFIRST:  " << eb->edges[j].timeFirst << endl;           
        }
    }
    cout << endl;
}

void stinger::print()
{
    cout << " Actual numNodes: " << num_nodes << 
            " numNodes initialized with: " << num_nodes_initialize << 
            " numEdges: " << num_edges << 
            " weighted: " << weighted << 
            " directed: " << directed << 
    endl;

    /*for(unsigned int i=0; i < num_nodes; i++){
        cout << "####################################################################" << endl;
        cout << "Node ID: "<< vertices[i].node<< " in-degree: "<< in_degree(vertices[i].node) << "  out-degree: " << out_degree(vertices[i].node) << endl;
        cout << "In-neighbors: " << endl;
        
        stinger_eb* eb = vertices[i].in_neighbors;
        
        while(eb!=nullptr){
            print_eb(eb);
            eb = eb->next;
        }
        cout << endl;

        cout << "Out-neighbors: " << endl;
        
        stinger_eb* eb1 = vertices[i].out_neighbors;
        
        while(eb1!=nullptr){
            print_eb(eb1);
            eb1 = eb1->next;
        }
        cout << endl;    

        cout << "####################################################################" << endl;  
        cout << endl;
    }*/
}

/*bool stinger::vertexExists(const Edge& e, bool source)
{
    bool exists;
    if(source) exists = e.sourceExists;
    else exists = e.destExists;

    // using CAS operations implemented in GAP 
    if(source){
        NodeID v = e.source;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    else{
        NodeID v = e.destination;
        bool aff = affected[v];
        if(!aff){
            compare_and_swap(affected[v], aff, true);
        }
    }
    
    if(exists){       
        stinger_int64_fetch_add(&num_edges, 1);         
        return true;
    }

    else{
        stinger_int64_fetch_add(&num_nodes, 1); 
        stinger_int64_fetch_add(&num_edges, 1);          
        return false;
    }  
}*/

/*void stinger::update(const EdgeList& el)
{
    #pragma omp parallel for 
    for(unsigned int i=0; i<el.size(); i++){
        bool exists = vertexExists(el[i], true); 
        if(!exists) updateForNewVertex(el[i], true);
        else updateForExistingVertex(el[i], true);
    
        // examine destination vertex 
        bool exists1 = vertexExists(el[i], false); 
        if(!exists1) updateForNewVertex(el[i], false);
        else updateForExistingVertex(el[i], false); 

    }             
}*/

/*void stinger::updateForNewVertex(const Edge& e, bool source)
{    
    // make a new stinger_vertex and put it into vertices array  
    NodeID n, dest;
    if(source) {n = e.source; dest = e.destination;}
    else {n = e.destination; dest = e.source;}
    Weight weight = e.weight;

    stinger_vertex* node = &vertices[n];     
    
    // take care of node.out_neighbors 
    stinger_eb* old_out_eb = (stinger_eb*)readfe((int64_t*)&(node->out_neighbors));
    if(!old_out_eb){
        // create a new out edge block 
        stinger_eb* newBlock = new stinger_eb(n);
        
        if(source || (!source && !directed)){
            // Add edge to the first slot in the new edge block 
            update_edge_data(newBlock, 0, dest, weight, false);
        }

        writeef((int64_t*)&(node->out_neighbors), *((int64_t*)&newBlock));   
    }else{
        // Some other thread already created a block
        // unlock and do the normal search and insert edge
        writeef((int64_t*)&(node->out_neighbors), *((int64_t*)&old_out_eb));  
        if(source || (!source && !directed)){
            search_and_insert_edge(e, source, node->out_neighbors, false);
        }
    }

    // take care of node.in_neighbors 
    stinger_eb* old_in_eb = (stinger_eb*)readfe((int64_t*)&(node->in_neighbors));
    if(!old_in_eb){
        // create a new out edge block 
        stinger_eb* newBlock = new stinger_eb(n);
        
        if(!source && directed){
            // Add edge to the first slot in the new edge block 
            update_edge_data(newBlock, 0, dest, weight, true);
        }

        writeef((int64_t*)&(node->in_neighbors), *((int64_t*)&newBlock));   
    }else{
        // Some other thread already created a block,
        // unlock and do the normal search and insert edge 
        writeef((int64_t*)&(node->in_neighbors), *((int64_t*)&old_in_eb));  
        if(!source && directed){
            search_and_insert_edge(e, source, node->in_neighbors, true);
        }
    }
}*/

/*void stinger::updateForExistingVertex(const Edge& e, bool source)
{
    NodeID n;
    if(source) n = e.source;
    else n = e.destination;
    stinger_vertex* node = &vertices[n];

    if(source || (!source && !directed)){        
        search_and_insert_edge(e, source, node->out_neighbors, false);             
    }
    else if(!source && directed){        
        search_and_insert_edge(e, source, node->in_neighbors, true);             
    }   
}*/