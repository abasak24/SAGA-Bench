#ifndef DYN_BFS_H_
#define DYN_BFS_H_

#include <algorithm>

#include "traversal.h"
#include "../common/timer.h"
#include "sliding_queue_dynamic.h"
#include "../common/pvector.h"

/* Algorithm: Incremental BFS and BFS starting from scratch */

template<typename T> 
void BFSIter0(T* ds, SlidingQueue<NodeID>& queue){  
    pvector<bool> visited(ds->num_nodes, false);     
  
    #pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for schedule(dynamic, 64)
        for(NodeID n=0; n < ds->num_nodes; n++){
            if(ds->affected[n]){
                float old_depth = ds->property[n];
                float new_depth = std::numeric_limits<float>::max();

                // pull new depth from incoming neighbors
                for(auto v: in_neigh(n, ds)){
                    if (ds->property[v] != -1) {
                        new_depth = std::min(new_depth, ds->property[v] + 1);
                    }
                }                                         
                
                // trigger happens if it is:
                // 1) brand new vertex with old_prop = -1 and we found a new valid min depth 
                // 2) already existing vertex and we found a new depth smaller than old depth 
                bool trigger = (
                ((new_depth < old_depth) || (old_depth == -1)) 
                && (new_depth != std::numeric_limits<float>::max())                 
                );               

                /*if(trigger){                                                 
                    ds->property[n] = new_depth; 
                    for(auto v: out_neigh(n, dataStruc, ds, directed)){
                        float curr_depth = ds->property[v];
                        float updated_depth = ds->property[n] + 1;                        
                        if((updated_depth < curr_depth) || (curr_depth == -1)){   
                            if(compare_and_swap(ds->property[v], curr_depth, updated_depth)){                                                              
                                lqueue.push_back(v); 
                            }
                        }
                    }
                }*/

                // Note: above is commented and included this new thing. 
                // Above was leading to vertices being queued redundantly
                // Above assumes updated_depth < curr_depth only once. 
                // This is not true in dynamic case because we start from affected vertices
                // whose depths are not all necessary the same.
                // In static version, the above works because static version starts from the source 
                // and we know that updated_depth < curr_depth only once. 

                if(trigger){
                    ds->property[n] = new_depth; 
                    for(auto v: out_neigh(n, ds)){
                        float curr_depth = ds->property[v];
                        float updated_depth = ds->property[n] + 1;
                        if((updated_depth < curr_depth) || (curr_depth == -1)){
                            bool curr_val = visited[v];
                            if(!curr_val){
                                if(compare_and_swap(visited[v], curr_val, true))
                                    lqueue.push_back(v);
                            }
                            while(!compare_and_swap(ds->property[v], curr_depth, updated_depth)){
                                curr_depth = ds->property[v];
                                if(curr_depth <= updated_depth){
                                    break;
                                }
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
void dynBFSAlg(T* ds, NodeID source){
    std::cout <<"Running dynamic BFS " << std::endl;
    
    Timer t;
    t.Start();
    
    SlidingQueue<NodeID> queue(ds->num_nodes);         
    if(ds->property[source] == -1) ds->property[source] = 0;
    
    BFSIter0(ds, queue);
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
                for(auto v: out_neigh(n, ds)){
                    float curr_depth = ds->property[v];
                    float new_depth = ds->property[n] + 1;
                    /*if((new_depth < curr_depth) || (curr_depth == -1)){
                        if(compare_and_swap(ds->property[v], curr_depth, new_depth)){                            
                            lqueue.push_back(v);
                        }
                    }*/

                    if((new_depth < curr_depth) || (curr_depth == -1)){
                        bool curr_val = visited[v];
                        if(!curr_val){
                            if(compare_and_swap(visited[v], curr_val, true))
                                    lqueue.push_back(v);
                        }

                        while(!compare_and_swap(ds->property[v], curr_depth, new_depth)){
                            curr_depth = ds->property[v];
                            if(curr_depth <= new_depth){
                                break;
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
void BFSStartFromScratch(T* ds, NodeID source){  
    //std::cout << "Source " << source << std::endl;
    std::cout << "Running BFS from scratch" << std::endl;

    Timer t;
    t.Start(); 

    #pragma omp parallel for 
    for(NodeID n = 0; n < ds->num_nodes; n++)
        ds->property[n] = -1;

    ds->property[source] = 0;    

    SlidingQueue<NodeID> queue(ds->num_nodes);   
    queue.push_back(source);
    queue.slide_window();  

    while(!queue.empty()){       
        //std::cout << "Queue not empty, Queue size: " << queue.size() << std::endl;         
        #pragma omp parallel
        {             
            QueueBuffer<NodeID> lqueue(queue);
            #pragma omp for 
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){
                NodeID u = *q_iter;
                for(auto v: out_neigh(u, ds)){
                    float curr_depth = ds->property[v];
                    float new_depth = ds->property[u] + 1;
                    if(curr_depth < 0){
                        if(compare_and_swap(ds->property[v], curr_depth, new_depth)){
                            lqueue.push_back(v);
                        }
                    }
                }
            }
            lqueue.flush();
        }
        queue.slide_window();        
    }

    t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();
}
#endif  // DYN_BFS_H_    
