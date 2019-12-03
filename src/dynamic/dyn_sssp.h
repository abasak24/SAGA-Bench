#ifndef DYN_SSSP_H_
#define DYN_SSSP_H_

#include <limits>
#include <vector>
#include <algorithm>

#include "traversal.h"
#include "sliding_queue_dynamic.h"
#include "../common/timer.h"
#include "../common/pvector.h"

/*
Algorithm: Incremental SSSP and SSSP from scratch
Vertex function from Keval Vora's TACO paper
*/

template<typename T> 
void SSSPIter0(T* ds, SlidingQueue<NodeID>& queue){   
    pvector<bool> visited(ds->num_nodes, false);   

    #pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
        #pragma omp for schedule(dynamic, 64)
        for(NodeID n=0; n < ds->num_nodes; n++){
            if(ds->affected[n]){                
                float old_path = ds->property[n];
                float new_path = kDistInf;
                
                neighborhood<T> neigh = in_neigh(n, ds);
                                 
                // pull new depth from incoming neighbors
                for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){                    
                    new_path = std::min(new_path, ds->property[*it] + it.extractWeight());
                }      

                bool trigger = (((new_path < old_path) && (new_path != kDistInf)));                 

                if(trigger){                   
                    ds->property[n] = new_path; 
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
void dynSSSPAlg(T* ds, NodeID source){    
    std::cout << "Running dynamic SSSP" << std::endl;
    Timer t;
    t.Start();

    SlidingQueue<NodeID> queue(ds->num_nodes);       
    
    // set all new vertices' rank to inf, otherwise reuse old values 
    #pragma omp parallel for schedule(dynamic, 64)
    for(NodeID n = 0; n < ds->num_nodes; n++){
        if(ds->property[n] == -1){
            if(n == source) ds->property[n] = 0;
            else ds->property[n] = kDistInf;
        }
    }      

    SSSPIter0(ds, queue); 
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
                float new_path = kDistInf;
                
                neighborhood<T> neigh = in_neigh(n, ds);
                                 
                // pull new depth from incoming neighbors
                for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){                    
                    new_path = std::min(new_path, ds->property[*it] + it.extractWeight());
                }      
                
                // valid depth + lower than before = trigger 
                bool trigger = (((new_path < old_path) && (new_path != kDistInf)));     

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
void SSSPStartFromScratch(T* ds, NodeID source, float delta){ 
    std::cout <<"Running SSSP from scratch" << std::endl;

    Timer t;
    t.Start();

    int num_edges_directed = ds->directed ? ds->num_edges : 2*ds->num_edges;
    
    #pragma omp parallel for 
    for(NodeID n = 0; n < ds->num_nodes; n++)
        ds->property[n] = kDistInf;    
    ds->property[source] = 0;

    pvector<NodeID> frontier(num_edges_directed);

    // two element arrays for double buffering curr=iter&1, next=(iter+1)&1
    size_t shared_indexes[2] = {0, kMaxBin};
    size_t frontier_tails[2] = {1, 0};
    frontier[0] = source;

    #pragma omp parallel
    {
        std::vector<std::vector<NodeID> > local_bins(0);
        size_t iter = 0;

        while (shared_indexes[iter&1] != kMaxBin) {
            size_t &curr_bin_index = shared_indexes[iter&1];
            size_t &next_bin_index = shared_indexes[(iter+1)&1];
            size_t &curr_frontier_tail = frontier_tails[iter&1];
            size_t &next_frontier_tail = frontier_tails[(iter+1)&1];
            #pragma omp for nowait schedule(dynamic, 64)
            for (size_t i=0; i < curr_frontier_tail; i++) {
                NodeID u = frontier[i];
                if (ds->property[u] >= delta * static_cast<float>(curr_bin_index)) {
                    neighborhood<T> neigh = out_neigh(u, ds);
                    for(neighborhood_iter<T> it = neigh.begin(); it != neigh.end(); it++){                    
                        float old_dist = ds->property[*it];
                        float new_dist = ds->property[u] + it.extractWeight();
                        if (new_dist < old_dist) {
                            bool changed_dist = true;
                            while (!compare_and_swap(ds->property[*it], old_dist, new_dist)){
                                old_dist = ds->property[*it];
                                if (old_dist <= new_dist) {
                                    changed_dist = false;
                                    break;
                                }
                            }
                            if (changed_dist) {
                                size_t dest_bin = new_dist/delta;
                                if (dest_bin >= local_bins.size()) {
                                    local_bins.resize(dest_bin+1);
                                }
                                local_bins[dest_bin].push_back(*it);
                            }
                        }
                    }
                }
            }
            for (size_t i=curr_bin_index; i < local_bins.size(); i++) {
                if (!local_bins[i].empty()) {
                    #pragma omp critical
                    next_bin_index = std::min(next_bin_index, i);
                    break;
                }
            }
            #pragma omp barrier
            #pragma omp single nowait
            {
                //t.Stop();
                //PrintStep(curr_bin_index, t.Millisecs(), curr_frontier_tail);
                //t.Start();
                curr_bin_index = kMaxBin;
                curr_frontier_tail = 0;
            }
            if (next_bin_index < local_bins.size()) {
                size_t copy_start = fetch_and_add(next_frontier_tail,
                    local_bins[next_bin_index].size());
                std::copy(local_bins[next_bin_index].begin(),
                    local_bins[next_bin_index].end(), frontier.data() + copy_start);
                local_bins[next_bin_index].resize(0);
            }
            iter++;
            #pragma omp barrier
        }
        //#pragma omp single
        //std::cout << "took " << iter << " iterations" << std::endl;
    }    

    t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();
}
#endif  // DYN_SSSP_H_