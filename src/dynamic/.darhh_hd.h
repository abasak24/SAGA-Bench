#ifndef HIGH_DEGREE_HPP
#define HIGH_DEGREE_HPP

#include <cmath>
#include <memory>

#include "types.h"

#include "rhh.h"

/*
  Robin Hood Has Map for High-Degree Vertices
  Implementation

  Author: Ryan Lorica
*/

template <typename T> class edge_chunk;
template <typename T> class hd_rhh;
template <typename T> class hd_iter;
template <typename T> class neighborhood;
template <typename T> class neighborhood_iter;
template <typename T> class darhh;
template <typename T>
std::ostream& operator<<(std::ostream &out, edge_chunk<T> const &ec);
template <typename T>
std::ostream& operator<<(std::ostream &out, hd_rhh<T> const &hd);

template <typename T>
class edge_chunk: public rhh<NodeID, T> {
    friend class hd_rhh<T>;
    friend class hd_iter<T>;
    friend class neighborhood<darhh<T>>;
    friend class neighborhood_iter<darhh<T>>;
    friend std::ostream& operator << <>(std::ostream &out, edge_chunk const &ec);
private:
    using super = rhh<NodeID, T>;
public:
    edge_chunk(int cap, float load_factor):
	super(cap, load_factor) {}
    edge_chunk(int cap): edge_chunk(cap, 0.9) {}
    edge_chunk(): edge_chunk(pow(2, 10), 0.9) {}
    void insert_elem(NodeID const &id, Weight const &w);
    bool get_elem(NodeID const &id, Weight &w) const;
    std::string to_string() const;
};

template <typename T>
class hd_rhh: public rhh<NodeID, edge_chunk<T>> {
    friend class hd_iter<T>;
    friend class neighborhood<darhh<T>>;
    friend class neighborhood_iter<darhh<T>>;
    friend std::ostream& operator<< <>(std::ostream &out, hd_rhh<T> const &hd);
private:
    using super = rhh<NodeID, edge_chunk<T>>;
    int ec_cap;
    float ec_load_factor;
public:
    hd_rhh(int cap, float load_factor, int _ec_cap, float _ec_load_factor):
	super(cap, load_factor, edge_chunk<T>(_ec_cap, _ec_load_factor)),
	ec_cap(_ec_cap),
	ec_load_factor(_ec_load_factor) {}  
    hd_rhh(int cap, float load_factor): hd_rhh(cap, load_factor, pow(2, 10), 0.9) {}
    hd_rhh(int cap): hd_rhh(cap, 0.9) {}
    hd_rhh(): hd_rhh(pow(2, 10)) {}
    void insert_elem(Edge const &edge);
    void insert_elem(EdgeID const &id, Weight const &w);
    void delete_elem(EdgeID const &id);
    bool get_elem(EdgeID const &id, Weight &w) const;
    unsigned int get_degree(NodeID const &it) const;
    hd_iter<T> begin(NodeID id);
    hd_iter<T> end(NodeID id);
    std::string to_string() const;
};

template <typename T>
class hd_iter {
    friend class hd_rhh<T>;
private:
    edge_chunk<T>* ec;
    unsigned int ec_index;
    NodeID src_id;
    unsigned int begin_i;
    unsigned int cursor_i;
    T* cursor;
public:
    hd_iter(hd_rhh<T>* _rhh, NodeID _src_id);
    hd_iter& operator++();
    hd_iter& operator++(int);
    bool operator!=(const hd_iter &it);
    inline NodeID operator*();
    inline NodeID extractNodeID() const;
    inline Weight extractWeight() const;
};


template <typename T>
void edge_chunk<T>::insert_elem(NodeID const &id, Weight const &w)
{
    T node;
    node.setInfo(id, w);
    super::insert_elem(id, node);
}

// This method is unnecessary
template <typename T>
bool edge_chunk<T>::get_elem(NodeID const &id, Weight &w) const
{
    unsigned int loc;
    bool found = super::find_elem(id, loc);
    if (found)
	w = super::arr[loc].value.getWeight();
    return found;
}

template <typename T>
std::string edge_chunk<T>::to_string() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

template <typename T>
void hd_rhh<T>::insert_elem(Edge const &edge)
{
    insert_elem(EdgeID(edge.source, edge.destination), edge.weight);
}

template <typename T>
void hd_rhh<T>::insert_elem(EdgeID const &id, Weight const &w)
{
    unsigned int loc;
    // We traverse 3 times. Don't do that.
    if (!super::find_elem(id.first, loc)) {
	
	//edge_chunk<T> ec(ec_cap, ec_load_factor);
	super::insert_elem(id.first, ec);
	super::find_elem(id.first, loc);
	// Opportunity for optimization. We traverse twice:
	// once to insert, then again to find.
    }
    this->arr[loc].val.insert_elem(id.second, w);
}

template <typename T>
void hd_rhh<T>::delete_elem(EdgeID const &id)
{
    unsigned int loc;
    if (super::find_elem(id.first, loc)) {
	this->arr[loc].val.delete_elem(id.second);
	if (this->arr[loc].val.size == 0)
	    super::delete_elem(id.first);
    }
}

template <typename T>
bool hd_rhh<T>::get_elem(EdgeID const &id, Weight &w) const
{
    unsigned int loc;
    return super::find_elem(id.first,loc) ?
	this->arr[loc].val.get_elem(id.second, w) : false;
}

template <typename T>
hd_iter<T> hd_rhh<T>::begin(NodeID id)
{
    return hd_iter<T>(this, id);
}

template <typename T>
hd_iter<T> hd_rhh<T>::end(NodeID id)
{
    hd_iter<T> it(this, id);
    it.cursor = nullptr;
    return it;
}

template <typename T>
unsigned int hd_rhh<T>::get_degree(NodeID const &id) const
{
    unsigned int loc; 
    return super::find_elem(id, loc) ? this->arr[loc].val.get_size() : 0;
}

template <typename T>
std::string hd_rhh<T>::to_string() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

template <typename T>
hd_iter<T>::hd_iter(hd_rhh<T>* hd, NodeID _src_id):
  src_id(_src_id)
{
    if (hd->find_elem(src_id, ec_index)) {
	this->ec = &(hd->arr[ec_index].val);
	this->begin_i = 0;
	while (ec->arr[this->begin_i].empty ||
	       !ec->arr[this->begin_i].valid) {
	    ++begin_i;
	}
	this->cursor_i = begin_i;
	this->cursor = &(ec->arr[begin_i].val);
    } else {
	this->cursor = nullptr;
    }
}

template <typename T>
hd_iter<T>& hd_iter<T>::operator++()
{
    rhh_elem<NodeID, T> elem;
    do {
	if(++cursor_i >= ec->get_capacity()) {
	    cursor = nullptr;
	    return *this;
	}
	elem = ec->arr[cursor_i];
    } while (elem.empty || !elem.valid);
    cursor = &(ec->arr[this->cursor_i].val);
    return *this;
}

template <typename T>
hd_iter<T>& hd_iter<T>::operator++(int)
{
    ++(*this);
    return *this;
}

template <typename T>
bool hd_iter<T>::operator!=(const hd_iter<T> &it)
{
    return this->cursor != it.cursor;
}

template <typename T>
NodeID hd_iter<T>::operator*()
{
    return cursor->getNodeID();
}

template <typename T>
NodeID hd_iter<T>::extractNodeID() const
{
    return cursor->getNodeID();
}

template <typename T>
Weight hd_iter<T>::extractWeight() const
{
    return cursor->getWeight();
}

template <typename T>
std::ostream& operator<<(std::ostream &out, edge_chunk<T> const &ec)
{
    out << "(";
    for (unsigned int i = 0; i < ec.get_capacity(); i++) {
	out << "edge_chunk[" << i << "]=";
	if (!ec.arr[i].empty) {
	    NodeID id = ec.arr[i].val.getNodeID();
	    Weight wt = ec.arr[i].val.getWeight();
	    out << "("
		<< "key=" << ec.arr[i].key << ","
		<< "val=(id=" << id << ",weight=" << wt << "),"
		<< "probe_dist=" << ec.arr[i].probe_dist << ","
		<< "valid=" << ec.arr[i].valid << ","
		<< "empty=" << ec.arr[i].empty
		<< ")";
	} else {
	    out << "null";
	}
	if (i < ec.get_capacity() - 1)
	    out << ",\n\t    ";
    }
    out << ")";
    return out; 
}

template <typename T>
std::ostream& operator<<(std::ostream &out, hd_rhh<T> const &hd)
{
    out << "\n(";
    for (unsigned int i = 0; i < hd.get_capacity(); i++) {
	out << "high_degree[" << i << "]=";
	if (hd.arr[i].val.get_size() > 0) {
	    out << "\n\t("
		<< "key=" << hd.arr[i].key << ","
		<< "\n\tval=" << hd.arr[i].val << ","
		<< "\n\tprobe_dist=" << hd.arr[i].probe_dist << ","
		<< "\n\tvalid=" << hd.arr[i].valid << ","
		<< "\n\tempty=" << hd.arr[i].empty
		<< ")";
	} else {
	    out << "null";
	}
	if(i < hd.get_capacity() - 1)
	    out << ",\n";
    }
    out << ")\n";
    return out; 
}

#endif
