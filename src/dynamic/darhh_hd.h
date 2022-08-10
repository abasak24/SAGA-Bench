#ifndef DARHH_HD_H
#define DARHH_HD_H

#include <cmath>
#include <memory>
#include <unordered_map>
#include <utility>

#include "types.h"
#include "rhh.h"

template <typename T> class darhh;
template <typename T> class neighborhood_iter;
template <typename T> class neighborhood;

template <typename T>
class hd_rhh {
    friend class neighborhood_iter<darhh<T>>;
private:
    using hashmap = std::unordered_map<NodeID, std::unique_ptr<rhh<NodeID, T>>>;
    using edge_chunk = rhh<NodeID, T>;
    class iter {
	friend class hd_rhh;
	friend class neighborhood<darhh<T>>;
	friend class neighborhood_iter<darhh<T>>;
    private:
	edge_chunk* ec;
	NodeID src;
	uint32_t pos;
	T* cursor;
    public:
	iter(NodeID src): src(src) {}
	iter() = default;
	iter& operator=(const iter &other);
	bool operator==(const iter &other) const;
	bool operator!=(const iter &other) const;
	void operator++();
    };
    uint32_t ec_cap;
    float load_factor;
    hashmap v_table;
public:
    hd_rhh(uint32_t ec_cap, float ec_load_factor):
	ec_cap(ec_cap),
	load_factor(ec_load_factor) {}
    hd_rhh(uint32_t ec_cap): hd_rhh(ec_cap, 0.9) {}
    hd_rhh(): hd_rhh(pow(2, 5)) {}
    void insert_elem(Edge edge);
    void insert_elem(EdgeID id, Weight w);
    bool delete_elem(EdgeID const &edge);
    inline uint32_t get_degree(NodeID const &id) const;
    iter begin(NodeID const &id) const;
    iter end(NodeID const &id) const;
};

template <typename T>
typename hd_rhh<T>::iter& hd_rhh<T>::iter::operator=(const iter &other)
{
    src = other.src;
    pos = other.pos;
    cursor = other.cursor;
    ec = other.ec;
    return *this;
}

template <typename T>
bool hd_rhh<T>::iter::operator==(const iter &other) const
{
    return cursor == other.cursor;
}

template <typename T>
bool hd_rhh<T>::iter::operator!=(const iter &other) const
{
    return cursor != other.cursor;
}

template <typename T>
void hd_rhh<T>::iter::operator++()
{
    if (++pos >= ec->get_capacity()) {
	cursor = nullptr;
	return;
    }
    while (ec->arr[pos].empty() ||
	   ec->arr[pos].deleted())
    {
	if (++pos >= ec->get_capacity()) {
	    cursor = nullptr;
	    return;
	}
    }
    cursor = &(ec->arr[pos].val);
}

template <typename T>
void hd_rhh<T>::insert_elem(Edge edge)
{
    insert_elem(EdgeID(edge.source, edge.destination), edge.weight); 
}

template <typename T>
void hd_rhh<T>::insert_elem(EdgeID id, Weight w)
{
    T node;
    node.setInfo(id.second, w);
    edge_chunk* ec;
    const auto& ret = v_table.find(id.first);
    if (ret == v_table.end()) {
	std::unique_ptr<edge_chunk> new_ec(new edge_chunk(ec_cap, load_factor));
	ec = v_table.insert(std::make_pair(id.first, std::move(new_ec))).first->second.get();
    } else {
	ec = ret->second.get();
    }
    ec->insert_elem(id.second, node);
}

template <typename T>
bool hd_rhh<T>::delete_elem(EdgeID const &id)
{
    const auto& ret = v_table.find(id.first);
    if (ret == v_table.end())
	return false;
    else
	return ret->second->delete_elem(id.second);
}

template <typename T>
uint32_t hd_rhh<T>::get_degree(NodeID const &id) const
{
    const auto& ret = v_table.find(id);
    if (ret == v_table.end())
	return 0;
    else
	return ret->second->get_size();
}

template <typename T>
typename hd_rhh<T>::iter hd_rhh<T>::begin(NodeID const &id) const
{
    iter it;
    it.pos = 0;
    it.src = id;
    const auto& ret = v_table.find(id);
    if (ret == v_table.end()) {
	it.cursor = nullptr;
    } else {
	it.ec = ret->second.get();
	if (it.ec->arr[it.pos].empty() ||
	    it.ec->arr[it.pos].deleted())
	    ++it;
	else
	    it.cursor = &(it.ec->arr[it.pos].val);
    }
    return it;
}

template <typename T>
typename hd_rhh<T>::iter hd_rhh<T>::end(NodeID const &id) const
{
    (void) id;
    iter it;
    it.cursor = nullptr;
    return it;
}

#endif
