#ifndef LOW_DEGREE_H
#define LOW_DEGREE_H

#include <cmath>
#include <iostream>
#include <string>

#include "types.h"
#include "rhh.h"

/*
  Robin Hood Hash Map for Low-Degree Vertices
  Implementation

  Author: Ryan Lorica
*/

template <typename T> class ld_rhh;
template <typename T> class darhh;
template <typename T> class neighborhood;
template <typename T> class neighborhood_iter;
template <typename T>
std::ostream& operator<<(std::ostream &out, ld_rhh<T> const &ld);

template <typename T>
class ld_rhh: public rhh<EdgeID, T> {
    friend class darhh<T>;
    friend class neighborhood_iter<darhh<T>>;
    friend std::ostream& operator<< <>(std::ostream &out, ld_rhh const &ld);
private:
    using super = rhh<EdgeID, T>;
    class iter {
	friend class ld_rhh;
	friend class darhh<T>;
	friend class neighborhood<darhh<T>>;
	friend class neighborhood_iter<darhh<T>>;
    private:
	NodeID src;
	uint32_t origin;
	uint32_t pos;
	T *cursor;
	ld_rhh *parent;
    public:
	iter(NodeID src, ld_rhh* parent): src(src), parent(parent) {}
	iter() = default;
	iter& operator=(const iter &other);
	bool operator==(const iter &other) const;
	bool operator!=(const iter &other) const;
	void operator++();
    };
public:
    ld_rhh(int cap, float rehash): rhh<EdgeID, T>(cap, rehash) {}
    ld_rhh(int cap): ld_rhh(cap, 0.9) {}
    ld_rhh(): ld_rhh(pow(2, 20), 0.9) {}
    void insert_elem(Edge const &edge);
    uint32_t get_degree(NodeID const &id) const;
    iter begin(NodeID id);
    iter end(NodeID id);
    std::string to_string() const;
};

template <typename T>
typename ld_rhh<T>::iter& ld_rhh<T>::iter::operator=(const iter &other)
{
    src = other.src;
    origin = other.origin;
    pos = other.pos;
    cursor = other.cursor;
    parent = other.parent;
    return *this;
}

template <typename T>
bool ld_rhh<T>::iter::operator==(const iter &other) const
{
    return cursor == other.cursor;
}

template <typename T>
bool ld_rhh<T>::iter::operator!=(const iter &other) const
{
    return cursor != other.cursor;
}

template <typename T>
void ld_rhh<T>::iter::operator++()
{
    for (;;) {
	pos = (pos + 1) % parent->get_capacity();
	uint32_t pd = parent->probe_dist(origin, pos);
	auto elem = parent->arr[pos];
	if (pd == parent->get_capacity() ||
	    elem.empty() ||
	    pd > parent->probe_dist(parent->desired_pos(elem.hash), pos))
	{
	    cursor = nullptr;
	    break;
	} else if (elem.key.first == src && !elem.deleted()) {
	    cursor = &(parent->arr[pos].val);
	    break;
	}
    }
}

template <typename T>
void ld_rhh<T>::insert_elem(Edge const &edge)
{
    EdgeID id = EdgeID(edge.source, edge.destination);
    T node;
    node.setInfo(edge.destination, edge.weight);
    super::insert_elem(id, node);
}

template <typename T>
uint32_t ld_rhh<T>::get_degree(NodeID const &id) const
{
    uint32_t origin = id % this->arr.capacity();
    uint32_t pos;
    uint32_t count = 0;
    for (uint32_t d = 0; d < this->arr.capacity(); ++d) {
	pos = (origin + d) % this->arr.capacity();
	auto elem = this->arr[pos];
	if (elem.empty())
	    break;
	else if (d > super::probe_dist(super::desired_pos(elem.hash), pos))
	    break;
	else if (elem.key.first == id && !elem.deleted())
	    ++count;
    }
    return count;
}

template <typename T>
typename ld_rhh<T>::iter ld_rhh<T>::begin(NodeID id)
{
    iter it = iter(id, this);
    it.origin = id % this->arr.capacity();
    it.pos = it.origin;
    it.src = id;
    auto elem = this->arr[it.pos];
    if (elem.empty())
	it.cursor = nullptr;
    else if (elem.key.first != id || elem.deleted())
	++it;
    else
	it.cursor = &(this->arr[it.pos].val);
    return it;
}

template <typename T>
typename ld_rhh<T>::iter ld_rhh<T>::end(NodeID id)
{
    iter it(id, this);
    it.cursor = nullptr;
    return it;
}

template <typename T>
std::string ld_rhh<T>::to_string() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

template <typename T>
std::ostream& operator<<(std::ostream &out, ld_rhh<T> const &rhh)
{
    out << "\n(";
    for (uint32_t i = 0; i < rhh.get_capacity(); i++) {
	out << "low_degree[" << i << "]=";
	if (!rhh.arr[i].empty)
	    out << rhh.arr[i];
	else
	    out << "null";
	if (i < rhh.get_capacity() - 1)
	    out << ",\n";
    }
    out << ")\n";
    return out; 
}

#endif
