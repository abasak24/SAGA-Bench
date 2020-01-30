#ifndef DYN_MC_H_
#define DYN_MC_H_

#include <algorithm>

#include "traversal.h"
#include "../common/timer.h"
#include "sliding_queue_dynamic.h"
#include "../common/pvector.h"

/* Algorithm: Incremental Max computation and Max Computation starting from scratch */

template<typename T>
void MCIter0(T* ds, SlidingQueue<NodeID>& queue){
    pvector<bool> visited(ds->num_nodes, false);
    
    #pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for schedule(dynamic, 64)
        for(NodeID n=0; n < ds->num_nodes; n++){
            if(ds->affected[n]){
                float old_val = ds->property[n];
                float new_val = old_val;

                // calculate new value
                for(auto v: in_neigh(n, ds)){
                    new_val = std::max(new_val, ds->property[v]);
                }
                
                assert(new_val >= old_val);

                ds->property[n] = new_val;                                
                bool trigger = (
                    (ds->property[n] > old_val)
                    || (old_val == n)
                ); 

                if(trigger){                   
                    //put the out-neighbors into active list 
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
}

template<typename T>
void dynMCAlg(T* ds){
    //std::cout << "Number of nodes: "<< ds->num_nodes << std::endl;   
    std::cout << "Running dynamic MC" << std::endl;
    Timer t;
    t.Start();   

    SlidingQueue<NodeID> queue(ds->num_nodes);        
    
    // Assign value of newly added vertices
    #pragma omp parallel for schedule(dynamic, 64)
    for(NodeID n = 0; n < ds->num_nodes; n++){
        if(ds->property[n] == -1){
            ds->property[n] = n;
        }
    }        

    MCIter0(ds, queue);
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
                float old_val = ds->property[n];
                float new_val = old_val;

                // calculate new value
                for(auto v: in_neigh(n, ds)){
                    new_val = std::max(new_val, ds->property[v]);
                }

                assert(new_val >= old_val);

                ds->property[n] = new_val;                
                bool trigger = (ds->property[n] > old_val); 

                if(trigger){
                    for(auto v: out_neigh(n, ds)){  
                        bool curr_val = visited[v];
                        if(!curr_val){
                            if(compare_and_swap(visited[v], curr_val, true)) 
                                 lqueue.push_back(v);
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
void MCStartFromScratch(T* ds){ 
    std::cout << "Running MC from scratch" << std::endl;

    Timer t;
    t.Start();

    #pragma omp parallel for
    for (NodeID n=0; n < ds->num_nodes; n++)
       ds->property[n] = n;
    
    SlidingQueue<NodeID> queue(ds->num_nodes);   
    pvector<bool> visited(ds->num_nodes, false); 

    // first iteration: all active vertices 
    #pragma omp parallel
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for 
        for(NodeID n = 0; n < ds->num_nodes; n++){
            float old_val = ds->property[n];
            float new_val = old_val;

            for(auto v: in_neigh(n, ds)){
                new_val = std::max(new_val, ds->property[v]);
            }
            assert(new_val >= old_val);

            ds->property[n] = new_val;

            if(ds->property[n] != old_val){
                for(auto w: out_neigh(n, ds)){
                    bool curr_val = visited[w];
                    if(!curr_val){
                        if(compare_and_swap(visited[w], curr_val, true)) lqueue.push_back(w);
                    }
                }
            }
        }
        lqueue.flush();
    }
    queue.slide_window(); 

    // Next iterations: According to active vertices in queue 
    while(!queue.empty()){
        //std::cout << "Queue not empty, Queue size: " << queue.size() << std::endl;
        visited.fill(0);   

        #pragma omp parallel
        {
            QueueBuffer<NodeID> lqueue(queue);
            #pragma omp for
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){
                NodeID n = *q_iter;
                float old_val = ds->property[n];
                float new_val = old_val;

                for(auto v: in_neigh(n, ds)){
                    new_val = std::max(new_val, ds->property[v]);
                }
                assert(new_val >= old_val);

                ds->property[n] = new_val;                               

                if(ds->property[n] != old_val){
                    for(auto w: out_neigh(n, ds)){  
                        bool curr_val = visited[w];
                        if(!curr_val){
                            if(compare_and_swap(visited[w], curr_val, true)) 
                                 lqueue.push_back(w);
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

/*template<typename T> 
void MCStartFromScratch(const string& datatype, T* partition, bool directed){ 
    std::cout << "Running MC from scratch" << std::endl;
    #pragma omp parallel for
    for (NodeID n=0; n < partition->num_nodes; n++)
       partition->property[n] = n;
    
    int num_iter = 0;
    bool change = true;
    while(change){
        change = false;
        num_iter++;
        #pragma omp parallel for
        for(NodeID n = 0; n < partition->num_nodes; n++){
            float old_val = partition->property[n];
            float new_val = old_val;

            for(auto v: in_neigh(n, datatype, partition, partition->directed)){
                new_val = std::max(new_val, partition->property[v]);
            }

            assert(new_val >= old_val);

            partition->property[n] = new_val; 
            if(partition->property[n] != old_val) change = true;
        }
    }

    std::cout << "MCFromScratch took " << num_iter << " iterations" << std::endl;      
}*/
#endif  // DYN_MC_H_    
