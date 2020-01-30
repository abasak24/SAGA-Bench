#ifndef DYN_CC_H_
#define DYN_CC_H_

#include <random>

#include "traversal.h"
#include "../common/timer.h"
#include "sliding_queue_dynamic.h"
#include "../common/pvector.h"

/* Algorithm: Incremental CC and CC starting from scratch */

typedef float Component;

template<typename T>
void CCIter0(T* ds, SlidingQueue<NodeID>& queue){
    pvector<bool> visited(ds->num_nodes, false);
    
    #pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for schedule(dynamic, 64)
        for(NodeID n=0; n < ds->num_nodes; n++){
            if(ds->affected[n]){
                Component old_comp = ds->property[n];
                Component new_comp = old_comp;

                // calculate new component
                for(auto v: in_neigh(n, ds)){
                    if(ds->property[v] < new_comp) new_comp = ds->property[v];
                }

                if(ds->directed){                    
                    for(auto v: out_neigh(n, ds)){
                        if(ds->property[v] < new_comp) new_comp = ds->property[v];
                    }
                }

                assert(new_comp<= old_comp);

                ds->property[n] = new_comp;                                
                bool trigger = ((ds->property[n] < old_comp) || (old_comp == n)); 

                if(trigger){                   
                    //put the out-neighbors into active list 
                    for(auto v: in_neigh(n, ds)){                        
                        bool curr_val = visited[v];
                        if(!curr_val){
                            if(compare_and_swap(visited[v], curr_val, true)) 
                                 lqueue.push_back(v);
                        }                       
                    }

                    if(ds->directed){
                        for(auto v: out_neigh(n, ds)){
                            bool curr_val = visited[v];
                            if(!curr_val){
                                if(compare_and_swap(visited[v], curr_val, true)) 
                                     lqueue.push_back(v);
                            }             
                        }
                    }                                                
                }        
            }
        }
        lqueue.flush();
    }   
}

template<typename T>
void dynCCAlg(T* ds){
    std::cout << "Running dynamic CC" << std::endl;

    Timer t;
    t.Start();

    SlidingQueue<NodeID> queue(ds->num_nodes);      
    
    // Assign component of newly added vertices
    #pragma omp parallel for schedule(dynamic, 64)
    for(NodeID n = 0; n < ds->num_nodes; n++){
        if(ds->property[n] == -1){
            ds->property[n] = n;
        }
    }    
   
    CCIter0(ds, queue);
    queue.slide_window();   
    
    while(!queue.empty()){             
        //std::cout << "Queue not empty, Queue size: " << queue.size() << std::endl;
        pvector<bool> visited(ds->num_nodes, false);         
       
        #pragma omp parallel
        {
            QueueBuffer<NodeID> lqueue(queue);
            #pragma omp for schedule(dynamic, 64)
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){
                NodeID n = *q_iter;
                Component old_comp = ds->property[n];
                Component new_comp = old_comp;

                // calculate new component
                for(auto v: in_neigh(n, ds)){
                    if(ds->property[v] < new_comp) new_comp = ds->property[v];
                }

                if(ds->directed){
                    for(auto v: out_neigh(n, ds)){
                        if(ds->property[v] < new_comp) new_comp = ds->property[v];
                    }
                }

                assert(new_comp<= old_comp);
                ds->property[n] = new_comp;                
                bool trigger = (ds->property[n] < old_comp); 

                if(trigger){
                    for(auto v: in_neigh(n, ds)){  
                        bool curr_val = visited[v];
                        if(!curr_val){
                            if(compare_and_swap(visited[v], curr_val, true)) 
                                   lqueue.push_back(v);
                        }                                                                    
                    }

                    if(ds->directed){
                        for(auto v: out_neigh(n, ds)){  
                            bool curr_val = visited[v];
                            if(!curr_val){
                                if(compare_and_swap(visited[v], curr_val, true)) 
                                      lqueue.push_back(v);
                            }                                                                   
                        }
                    }
                }
            }
            lqueue.flush();
        }
        queue.slide_window();            
    }    

    // clear affected array to get ready for the next update round
    #pragma omp parallel for schedule(dynamic, 64)
    for(NodeID i = 0; i < ds->num_nodes; i++){
        ds->affected[i] = false;
    }        

    t.Stop();
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();
}

template<typename T>
void CCStartFromScratch(T* ds){ 
    //std::cout << "Number of nodes: "<< ds->num_nodes << std::endl;
    std::cout << "Running CC from scratch" << std::endl;

    Timer t;
    t.Start();

    #pragma omp parallel for
    for (NodeID n=0; n < ds->num_nodes; n++)
       ds->property[n] = n;   

    bool change = true;
    int num_iter = 0;  
    
    if(ds->directed){
        while (change){
            change = false;
            num_iter++;
            #pragma omp parallel for
            for (NodeID u=0; u < ds->num_nodes; u++) {
                for (NodeID v : out_neigh(u, ds)){
                    NodeID comp_u = ds->property[u];
                    NodeID comp_v = ds->property[v];
                    if (comp_u == comp_v) continue;
                    // Hooking condition so lower component ID wins independent of direction
                    NodeID high_comp = comp_u > comp_v ? comp_u : comp_v;
                    NodeID low_comp = comp_u + (comp_v - high_comp);
                    if (high_comp == ds->property[high_comp]) {
                        change = true;
                        ds->property[high_comp] = low_comp;
                    }
                }
            }

            #pragma omp parallel for
            for (NodeID n=0; n < ds->num_nodes; n++){
                while (ds->property[n] != ds->property[ds->property[n]]){
                    ds->property[n] = ds->property[ds->property[n]];
                }
            }
        }
    }
    else{
        while (change) {
            change = false;
            num_iter++;
            #pragma omp parallel for
            for (NodeID u=0; u < ds->num_nodes; u++) {
                NodeID comp_u = ds->property[u];
                for (NodeID v : out_neigh(u, ds)) {
                    NodeID comp_v = ds->property[v];
                    // To prevent cycles, we only perform a hook in a consistent direction
                    // (comp_u < comp_v). Since the graph is undirected, the condition
                    // will be true from one side.
                    if ((comp_u < comp_v) && (comp_v == ds->property[comp_v])) {
                        change = true;
                        ds->property[comp_v] = comp_u;
                    }
                }
            }

            #pragma omp parallel for
            for (NodeID n=0; n < ds->num_nodes; n++) {
                while (ds->property[n] != ds->property[ds->property[n]]) {
                    ds->property[n] = ds->property[ds->property[n]];
                }
            }
        }
    }   

    t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();

    //std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;         
}
#endif  // DYN_CC_H_    