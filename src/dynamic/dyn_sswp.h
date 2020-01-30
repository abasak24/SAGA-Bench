#ifndef DYN_SSWP_H_
#define DYN_SSWP_H_

#include <vector>
#include <algorithm>

#include "traversal.h"
#include "sliding_queue_dynamic.h"
#include "../common/pvector.h"

/* Algorithm: Incremental SSWP and SSWP from scratch. 
This is the bottleneck shortest path problem. 
*/

template<typename T> 
void SSWPIter0(T* ds, SlidingQueue<NodeID>& queue){   
    pvector<bool> visited(ds->num_nodes, false);   

    #pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for schedule(dynamic, 64)
        for(NodeID n=0; n < ds->num_nodes; n++){
            if(ds->affected[n]){                
                float old_path = ds->property[n];
                std::vector<float> arr;                
                
                neighborhood<T> neigh = in_neigh(n, ds);
                float temp;
                                 
                // prepare arr vector 
                for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){                    
                    temp = std::min(ds->property[*it], static_cast<float>(it.extractWeight()));
                    arr.push_back(temp);
                }      

                if(!arr.empty()){
                    // find max in arr vector 
                    float new_path = arr[0];
                    for(std::vector<float>::iterator it = arr.begin(); it!=arr.end(); it++){
                        new_path = std::max(new_path, *it);
                    }

                    bool trigger = (new_path > old_path);        

                    if(trigger){
                        ds->property[n] = new_path;
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
void dynSSWPAlg(T* ds, NodeID source){    
    std::cout << "Running dynamic SSWP" << std::endl;
    Timer t;
    t.Start();

    SlidingQueue<NodeID> queue(ds->num_nodes);       
    
    // set all new vertices' rank to inf, otherwise reuse old values 
    #pragma omp parallel for schedule(dynamic, 64)
    for(NodeID n = 0; n < ds->num_nodes; n++){
        if(ds->property[n] == -1){
            if(n == source) ds->property[n] = kDistInf;
            else ds->property[n] = 0;
        }
    }      

    SSWPIter0(ds, queue); 
    queue.slide_window();
    
    while(!queue.empty()){         
        //std::cout << "Not empty queue, Queue Size:" << queue.size() << std::endl;        
        pvector<bool> visited(ds->num_nodes, false); 

        #pragma omp parallel 
        {
            QueueBuffer<NodeID> lqueue(queue);   
            #pragma omp for schedule(dynamic, 64)
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){
                NodeID n = *q_iter;
                float old_path = ds->property[n];      
                std::vector<float> arr;                
                
                neighborhood<T> neigh = in_neigh(n, ds);
                float temp;
                                 
                // prepare arr vector 
                for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){                    
                    temp = std::min(ds->property[*it], static_cast<float>(it.extractWeight()));
                    arr.push_back(temp);
                }

                if(!arr.empty()){
                    // find max in arr vector 
                    float new_path = arr[0];
                    for(std::vector<float>::iterator it = arr.begin(); it!=arr.end(); it++){
                        new_path = std::max(new_path, *it);
                    }
                    
                    bool trigger = (new_path > old_path);        

                    if(trigger){
                        ds->property[n] = new_path;                         
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
void SSWPStartFromScratch(T* ds, NodeID source){ 
    std::cout <<"Running SSWP from scratch" << std::endl;

    Timer t;
    t.Start();

    #pragma omp parallel for 
    for(NodeID n = 0; n < ds->num_nodes; n++)
        ds->property[n] = 0;
    ds->property[source] = kDistInf;   

    SlidingQueue<NodeID> queue(ds->num_nodes);   
    queue.push_back(source);
    queue.slide_window();  

    while(!queue.empty()){       
        //std::cout << "Queue not empty, Queue size: " << queue.size() << std::endl;    
        pvector<bool> visited(ds->num_nodes, false);      
        #pragma omp parallel
        {             
            QueueBuffer<NodeID> lqueue(queue);
            #pragma omp for 
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){
                NodeID u = *q_iter;

                neighborhood<T> neigh = out_neigh(u, ds);
                for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){ 
                    NodeID v = *it;
                    float old_dist = ds->property[v];
                    float new_dist = std::min(ds->property[u], static_cast<float>(it.extractWeight()));

                    if (new_dist > old_dist){
                        bool curr_val = visited[v];
                        if(!curr_val){ 
                            if(compare_and_swap(visited[v], curr_val, true))
                                    lqueue.push_back(v);
                        }
                        
                        while (!compare_and_swap(ds->property[v], old_dist, new_dist)){
                            old_dist = ds->property[v];
                            if (old_dist >= new_dist) break;                            
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
#endif  // DYN_SSWP_H_

//INITIAL PULL VERSION WHICH ALSO WORKED
/*template<typename T> 
void SSWPStartFromScratch(const string& dataStruc, T* partition, bool directed, NodeID source){ 
    std::cout <<"Running SSWP from scratch" << std::endl;

    #pragma omp parallel for 
    for(NodeID n = 0; n < partition->num_nodes; n++)
        partition->property[n] = 0;
    partition->property[source] = kDistInf;    

    pvector<bool> visited(partition->num_nodes, false); 
    SlidingQueue<NodeID> queue(partition->num_nodes); 
    
    // this is done by one thread 
    for(auto v:out_neigh(source, dataStruc, partition, directed)){
        if(!visited[v]){
            queue.push_back(v);
            visited[v] = true;
        }
    }

    queue.slide_window();

    while(!queue.empty()){       
        std::cout << "Queue not empty, Queue size: " << queue.size() << std::endl;         
        visited.fill(0);
        #pragma omp parallel
        {             
            QueueBuffer<NodeID> lqueue(queue);
            #pragma omp for 
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++){

                NodeID v = *q_iter;
                float old_dist = partition->property[v];
                float maxPath = 0;
                
                uint32_t num_in_neigh =0;
                neighborhood<T> neigh = in_neigh(v, dataStruc, partition, directed);
                for(typename neighborhood<T>::NeighborIterator it = neigh.begin(); it != neigh.end(); it++){                               
                    float p = std::min(partition->property[it.extractNodeID()], static_cast<float>(it.extractWeight()));                    
                    if(p>maxPath) maxPath = p;
                    num_in_neigh++;
                }
                
                bool gotMaxPath = (num_in_neigh >0);
                
                if(gotMaxPath && (maxPath > old_dist)){
                    partition->property[v] = maxPath;
                    // time to put its out neighbors as active vertices 
                    for(auto w: out_neigh(v, dataStruc, partition, directed)){
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
}*/