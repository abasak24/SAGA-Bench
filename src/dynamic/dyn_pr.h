#ifndef DYN_PR_H_
#define DYN_PR_H_

#include "traversal.h"
#include "../common/timer.h"
#include "sliding_queue_dynamic.h"
#include "../common/pvector.h"
#include <cmath>
#include <iostream>

/*
  Algorithm: Incremental PageRank and PageRank starting from scratch
*/

typedef float Rank;
const float kDamp = 0.85;
const float PRThreshold = 0.0000001;  

template<typename T> 
void PRIter0(T* ds, SlidingQueue<NodeID>& queue, Rank base_score)
{   
    pvector<Rank> outgoing_contrib(ds->num_nodes, 0);
    pvector<bool> visited(ds->num_nodes, false);
#pragma omp parallel for schedule(dynamic, 64)
    for(NodeID n=0; n < ds->num_nodes; n++) {    
        outgoing_contrib[n] = ds->property[n] / (ds->out_degree(n));      
    }

#pragma omp parallel     
    {
        QueueBuffer<NodeID> lqueue(queue);
#pragma omp for schedule(dynamic, 64)
        for (NodeID n=0; n < ds->num_nodes; n++) {
            if (ds->affected[n]) {
                Rank old_rank = ds->property[n];
                Rank incoming_total = 0;
                for(auto v: in_neigh(n, ds)){
                    incoming_total += outgoing_contrib[v];
                }
                    
                ds->property[n] = base_score + kDamp * incoming_total;                      
                bool trigger = fabs(ds->property[n] - old_rank) > PRThreshold; 
                if (trigger) {
                    //put the out-neighbors into active list 
                    for (auto v: out_neigh(n, ds)) {                        
                        bool curr_val = visited[v];
                        if (!curr_val) {
                            if (compare_and_swap(visited[v], curr_val, true)) 
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
void dynPRAlg(T* ds)
{     
    std::cout << "Running dynamic PR" << std::endl;  

    Timer t;
    t.Start();

    SlidingQueue<NodeID> queue(ds->num_nodes);       
    const Rank base_score = (1.0f - kDamp)/(ds->num_nodes); 
    // set all new vertices' rank to 1/num_nodes, otherwise reuse old values 
#pragma omp parallel for schedule(dynamic, 64)
    for (NodeID n = 0; n < ds->num_nodes; n++) {
        if (ds->property[n] == -1) {
            ds->property[n] = 1.0f/(ds->num_nodes);
        }
    } 

    // Iteration 0 only on affected vertices    
    PRIter0(ds, queue, base_score); 
    //cout << "Done iter 0" << endl;
    queue.slide_window();
    /*ofstream out("queueSizeParallel.csv", std::ios_base::app);   
    out << queue.size() << std::endl;
    std::cout << "Queue Size: " << queue.size() << std::endl;
    out.close();*/
    // Iteration 1 onward, process vertices in the queue 
    while (!queue.empty()) {         
        //std::cout << "Not empty queue, Queue Size:" << queue.size() << std::endl;
        pvector<Rank> outgoing_contrib(ds->num_nodes, 0);
        pvector<bool> visited(ds->num_nodes, false); 
        #pragma omp parallel for 
        for (NodeID n=0; n < ds->num_nodes; n++) { 
            outgoing_contrib[n] = ds->property[n]/(ds->out_degree(n));      
        }     
        #pragma omp parallel 
        {
            QueueBuffer<NodeID> lqueue(queue);   

            #pragma omp for schedule(dynamic, 64) 
            for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
                NodeID n = *q_iter;
                Rank old_rank = ds->property[n];
                Rank incoming_total = 0;
                for(auto v: in_neigh(n, ds))
                    incoming_total += outgoing_contrib[v];
                ds->property[n] = base_score + kDamp * incoming_total;                      
                bool trigger = fabs(ds->property[n] - old_rank) > PRThreshold; 
                if (trigger) {
                    //put the out-neighbors into active list 
                    for (auto v: out_neigh(n, ds)) {
                        bool curr_val = visited[v];
                        if (!curr_val) {
                            if (compare_and_swap(visited[v], curr_val, true)) 
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
    for (NodeID i = 0; i < ds->num_nodes; i++) {
        ds->affected[i] = false;
    }

     t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();
    //cout << "Done" << endl;    
}

template<typename T>
void PRStartFromScratch(T* ds)
{ 
    std::cout << "Running PR from scratch" << std::endl;

    Timer t;
    t.Start();

    const Rank base_score = (1.0f - kDamp)/(ds->num_nodes);
    int max_iters = 10;
    double epsilon = 0.0001;
    // Reset ALL property values 
#pragma omp parallel for
    for (NodeID n = 0; n < ds->num_nodes; n++) {
        ds->property[n] = 1.0f / (ds->num_nodes);        
    }
    pvector<Rank> outgoing_contrib(ds->num_nodes, 0);
    for (int iter = 0; iter < max_iters; iter++) {
        double error = 0;
#pragma omp parallel for
        for (NodeID n = 0; n < ds->num_nodes; n++) { 
            outgoing_contrib[n] = ds->property[n]/(ds->out_degree(n));      
        }
#pragma omp parallel for reduction(+ : error) schedule(dynamic, 64)
        for (NodeID u = 0; u < ds->num_nodes; u++) {
            Rank incoming_total = 0;
            for (NodeID v : in_neigh(u, ds))
		incoming_total += outgoing_contrib[v];
            Rank old_rank = ds->property[u];
            ds->property[u] = base_score + kDamp * incoming_total;
            error += fabs(ds->property[u] - old_rank);
        }
        //std::cout << "Epsilon: "<< epsilon << std::endl;
        //printf(" %2d    %lf\n", iter, error);
        if (error < epsilon)
	    break;
    } 

    t.Stop();    
    ofstream out("Alg.csv", std::ios_base::app);   
    out << t.Seconds() << std::endl;    
    out.close();
}

#endif // DYN_PR_H