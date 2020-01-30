#include <unistd.h>
#include <fstream>
#include <cstring>
#include <mutex>
#include <thread>

#include "builder.h"
#include "fileReader.h"
#include "topDataStruc.h"
#include "parser.h"

/* Main thread that launches everything else */

int main(int argc, char* argv[])
{    
    cmd_args opts = parse(argc, argv);
    ifstream file(opts.filename);
    if (!file.is_open()) {
        cout << "Couldn't open file " << opts.filename << endl;
	exit(-1);
    }    

    std::mutex q_lock;
    
    EdgeBatchQueue queue;
    bool loop = true;  
    dataStruc* struc = createDataStruc(opts.type, opts.weighted, opts.directed, opts.num_nodes, opts.num_threads);    
    std::thread t1(dequeAndInsertEdge, opts.type, struc, &queue, &q_lock, opts.algorithm, &loop);   
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);

    int rc = pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t), &cpuset);
    
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }

    int batch_id = 0;
    NodeID lastAssignedNodeID = -1;
    MapTable VMAP;

    while (!file.eof()) {        
        EdgeList el = readBatchFromCSV(
	    file,
	    opts.batch_size,
	    batch_id,
	    opts.weighted,
	    VMAP,
	    lastAssignedNodeID);
	q_lock.lock();     
        queue.push(el);
	q_lock.unlock();
	batch_id++;          
    }
    file.close();

    bool allEmpty = false;
    while (!allEmpty) {   
        q_lock.lock();
	allEmpty = queue.empty();
	q_lock.unlock();
	sleep(20);
    }
    
    loop = false;
    t1.join();
    
    //cout << "Started printing queues " << endl;
    //printEdgeBatchQueue(queue);
    //cout << "Done printing queues " << endl;    
    struc->print();
}