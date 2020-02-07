#include "builder.h"

#include <iostream>
#include <fstream>

#include "topAlg.h"
#include "topDataStruc.h"
#include "../common/timer.h"

void* dequeAndInsertEdge(
		std::string dtype,
		dataStruc *ds,
    EdgeBatchQueue *q,
    std::mutex *q_lock,
    std::string algorithm,
    bool *still_reading)
{	
	//std::cout << "Thread dequeAndInsertEdge: on CPU " << sched_getcpu() << "\n";
    Algorithm alg(algorithm, ds, dtype);
    int batch = 0;
    EdgeList el;
    q_lock->lock();
    while (*still_reading || !q->empty()) {		
	if (!q->empty()) {		
	    el = q->front();
	    q->pop();
	    q_lock->unlock();
		Timer t;
		t.Start();
		ds->update(el);	
		
		t.Stop();    
        ofstream out("Update.csv", std::ios_base::app);   
        out << t.Seconds() << std::endl;    
        out.close();	
	    std::cout << "Updated Batch: " << batch << std::endl;
	    batch++;
	    alg.performAlg();
	} else {		
	    q_lock->unlock();		
	    std::this_thread::sleep_for(std::chrono::milliseconds(1));		
	}
	q_lock->lock();
    }
    q_lock->unlock();
    
    // ##################### CORRECTNESS CHECK ############################
    // LJ: batch == 138
    // Orkut: batch == 235
    // Pokec: batch == 62
    // Wiki: batch == 58
    // 15_30m: batch == 61

    /*for (int64_t i = 0; i < ds->num_nodes; ++i) {
	std::cout << "Property[" << i << "] = "
		  << ds->property[i] << std::endl;
    }*/

    /*if ((algorithm == "prdyn") && (dtype == "adListChunked")) {
	ofstream out("PRDynAdListOrkut.csv"); 
	for(int64_t i =0; i < ds->num_nodes; i++){
	    out << ds->property[i] << endl;
	}                       
	out.close(); 
    } else if ((algorithm == "prfromscratch") && (dtype == "adListChunked")) {
	ofstream out("PRStatAdListOrkut.csv"); 
	for(int64_t i =0; i < ds->num_nodes; i++){
	    out << ds->property[i] << endl;
	}                       
	out.close();
    } else if ((algorithm == "prdyn") && (dtype == "degAwareRHH")) {
	ofstream out("PRDynDarhhOrkut.csv"); 
	for(int64_t i =0; i < ds->num_nodes; i++){
	    out << ds->property[i] << endl;
	}                       
	out.close();
    } else if ((algorithm == "prfromscratch") && (dtype == "degAwareRHH")) {
	ofstream out("PRStatDarhhOrkut.csv");
	for(int64_t i =0; i < ds->num_nodes; i++){
	    out << ds->property[i] << endl;
	}                       
	out.close();
    }*/

    return 0;
}
