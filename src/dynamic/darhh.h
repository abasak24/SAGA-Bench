#ifndef DARHH_H
#define DARHH_H

#include <algorithm>
#include <chrono>
#include <queue>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "abstract_data_struc.h"

#include "darhh_ld.h"
#include "darhh_hd.h"

/*
  Data Structure: Degree-Aware RHH
  Author: Ryan Lorica
*/

template <typename U> class neighborhood;
template <typename U> class neighborhood_iter;

template <typename T>
class darhh: public dataStruc {
    friend neighborhood<darhh<T>>;
    friend neighborhood_iter<darhh<T>>;
private:
    using super = dataStruc;
    class partition {
	friend darhh<T>;
	friend neighborhood<darhh<T>>;
	friend neighborhood_iter<darhh<T>>;
    private:
	void transfer_low_to_high(NodeID const &n);
	void insert(Edge const &e);
	std::queue<Edge> q;
	std::mutex q_mutex;
    public:
	partition(darhh* parent);
	partition() = delete;
	~partition();
	inline void enqueue(Edge const &e);
	darhh* parent;
	ld_rhh<T>* ld;
	hd_rhh<T>* hd;
    };
    static void dequeue_loop(partition *pt, volatile bool& done);
    inline int32_t pt_hash(NodeID const &n) const;
    static const int8_t ld_threshold = 2;
    const int64_t init_num_nodes;
    const int64_t num_out_partitions;
    const int64_t num_in_partitions;
    std::vector<std::unique_ptr<partition>> in, out;
    std::mutex affected_mutex, num_nodes_mutex;
public:
    darhh(bool w, bool d, int64_t init_nn, int64_t nt);
    void update(EdgeList const &el) override;
    int64_t out_degree(NodeID n) override;
    int64_t in_degree(NodeID n) override;
    void print() override;
    std::string to_string() const;
};

template <typename T>
darhh<T>::partition::partition(darhh* parent):
    parent(parent)
{
    ld = new ld_rhh<T>();
    hd = new hd_rhh<T>();
}

template <typename T>
darhh<T>::partition::~partition()
{
    delete ld;
    delete hd;
}

template <typename T>
void darhh<T>::partition::transfer_low_to_high(NodeID const &n)
{
    for (auto it = ld->begin(n), end = ld->end(n); it != end; ++it) {
	EdgeID id(n, it.cursor->getNodeID());
	hd->insert_elem(id, it.cursor->getWeight());
	ld->arr[it.pos].mark_deleted();
    }
}

template <typename T>
void darhh<T>::partition::insert(Edge const &e)
{
    if (!e.sourceExists) {
	ld->insert_elem(e);
	//std::lock_guard<std::mutex> guard(parent->num_nodes_mutex);
	//++parent->super::num_nodes;
    } else {
	int deg = ld->get_degree(e.source);
	if (deg > 0 && deg < ld_threshold) {
	    ld->insert_elem(e);
	} else if (deg == ld_threshold) {
	    transfer_low_to_high(e.source);
	    hd->insert_elem(e);
	} else {
	    if (hd->get_degree(e.source) > 0)
		hd->insert_elem(e);
	    else
		ld->insert_elem(e);
	}
    }
    //std::lock_guard<std::mutex> guard(parent->affected_mutex);
    //parent->super::affected[e.source] = 1;
}

template <typename T>
void darhh<T>::partition::enqueue(Edge const &e)
{
    std::lock_guard<std::mutex> guard(q_mutex);
    q.push(e);
}

template <typename T>
darhh<T>::darhh(bool w, bool d, int64_t init_nn, int64_t nt):
    super(w, d),
    init_num_nodes(init_nn),
    num_out_partitions(d ? nt / 2 : nt),
    num_in_partitions(d ? nt / 2 : 0)
{
    super::property.resize(init_num_nodes, -1);
    super::affected.resize(init_num_nodes);
    super::affected.fill(false);
    for (int i = 0; i < num_out_partitions; ++i) {
	out.push_back(std::unique_ptr<partition>(new partition(this)));
    }
    for (int i = 0; i < num_in_partitions; ++i) {
	in.push_back(std::unique_ptr<partition>(new partition(this)));
    }
}

template <typename T>
void darhh<T>::dequeue_loop(partition *pt, volatile bool &done)
{
    Edge e;
    pt->q_mutex.lock();
    while (!done || !pt->q.empty()) {
	if (!pt->q.empty()) {
	    e = pt->q.front();
	    pt->q.pop();
	    pt->q_mutex.unlock();
	    pt->insert(e);
	} else {
	    pt->q_mutex.unlock();
	    // Tune this.
	    std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	pt->q_mutex.lock();
    }
    pt->q_mutex.unlock();
}

template <typename T>
int32_t darhh<T>::pt_hash(NodeID const &n) const
{
    return n % num_out_partitions;
}

template <typename T>
void darhh<T>::update(EdgeList const &el)
{
    bool done = false;
    std::vector<std::unique_ptr<std::thread>> dqs;

    //#######.............Abanti added for thread pinning.............#########
    int64_t count = 2;
    //std::cout << "IN" << std::endl;
    for(auto& ptr: in){        
	//std::cout << "Count: " << count << " Cpu: " << count + 2 << std::endl;
	std::thread* t = new std::thread(dequeue_loop, ptr.get(), std::ref(done));
        dqs.push_back(std::unique_ptr<std::thread>(t));

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(count, &cpuset);

        int rc = pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
    
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }       
        //cout << "IN " << count << endl;
        count++; 
        //if(count == 6) count = 26;       
    }
        
    //std::cout << "OUT" << std::endl;
    for (auto& ptr: out){
	//std::cout << "Count: " << count << " Cpu: " << count + 2 << std::endl;
	std::thread* t = new std::thread(dequeue_loop, ptr.get(), std::ref(done));
        dqs.push_back(std::unique_ptr<std::thread>(t));

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(count, &cpuset);

        int rc = pthread_setaffinity_np(t->native_handle(), sizeof(cpu_set_t), &cpuset);
    
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }        
        //cout << "OUT " << count << endl;
        count++;
        //if(count == 6) count = 26;*/
    } 
    //#######.............Abanti added for thread pinning.............#########

    int o_ix, i_ix;
    for (auto& e: el) {
        affected[e.source] = true;
        affected[e.destination] = true;
        if (!e.sourceExists) num_nodes++;
        if (!e.destExists) num_nodes++;
	o_ix = pt_hash(e.source);
	out[o_ix]->enqueue(e);
	auto d = e.reverse();
    
	i_ix = pt_hash(d.source);

    
	if (dataStruc::directed)
	    in[i_ix]->enqueue(d);
	else
	    out[i_ix]->enqueue(d);
	++dataStruc::num_edges;
    ++dataStruc::num_edges; // entered two edges into system from a single edge 
    }
    done = true;
    for (auto& dq: dqs) {
	dq->join();
    }
}

template <typename T>
int64_t darhh<T>::in_degree(NodeID n)
{
    int32_t owner = pt_hash(n);
    int64_t degree;
    if (directed) {
	degree = in[owner]->ld->get_degree(n);
	if (degree == 0)
	    degree = in[owner]->hd->get_degree(n);
    } else {
	degree = out[owner]->ld->get_degree(n);
	if (degree == 0)
	    degree = out[owner]->hd->get_degree(n);
    }
    return degree;
}

template <typename T>
int64_t darhh<T>::out_degree(NodeID n)
{
    int32_t owner = pt_hash(n);
    int64_t degree = out[owner]->ld->get_degree(n);
    if (degree == 0)
	degree = out[owner]->hd->get_degree(n);
    return degree;
}


template <typename T>
std::string darhh<T>::to_string() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

template <typename T>
void darhh<T>::print()
{
    std::cout << " numNodes: " << super::num_nodes
	      << " numEdges: " << super::num_edges
	      << " weighted: " << super::weighted
	      << " directed: " << super::directed
	      << std::endl;
}

#endif
