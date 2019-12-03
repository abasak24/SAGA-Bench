#ifndef RHH_H
#define RHH_H

#include <cassert>
#include <cmath>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
  Robin Hood Hash Map
  Implementation
  
  Author: Ryan Lorica
*/

template <typename K, typename V>
class rhh_bsd {
protected:
    class elem {
    public:
	K key;
	V val;
	uint32_t hash;
	elem(K k, V v, uint32_t h):
	    key(k),
	    val(v),
	    hash(h) {}
	elem(K k, V v):
	    elem(k, v, 0) {}
	elem():
	    elem(K{}, V{}, 0) {}
	inline bool empty() const;
	std::string to_string();
    };
    inline uint32_t hash(K const &key) const;
    inline uint32_t desired_pos(uint32_t const &hash) const;
    inline uint32_t cyclic_dist(
	uint32_t const &origin,
	uint32_t const &cursor) const;
    inline void do_insert(
	uint32_t const &pos,
	uint32_t const &hash,
	K const &k,
	V const &v);
    bool find_pos(K const &key, uint32_t &i) const;
    void double_capacity();
    float load_factor;
    uint32_t size = 0;
public:
    std::vector<elem> arr; // This shouldn't be public. But screw it.
    rhh_bsd(int cap, float load_factor, K init_key, V init_val);
    rhh_bsd(int cap, float load_factor, V init_val):
	rhh_bsd(cap, load_factor, K{}, init_val) {}
    rhh_bsd(int cap, float load_factor):
	rhh_bsd(cap, load_factor, V{}) {}
    rhh_bsd(int cap):
	rhh_bsd(cap, 0.9) {}
    rhh_bsd():
	rhh_bsd(pow(2, 20)) {}
    rhh_bsd(rhh_bsd&) = delete;
    void insert_elem(K key, V val);
    bool delete_elem(K const &key);
    bool get_elem(K const &key, V &val) const;
    inline uint32_t get_capacity() const;
    inline uint32_t get_size() const;
    int32_t avg_cyclic_dist() const;
};

template <typename K, typename V>
bool rhh_bsd<K, V>::elem::empty() const
{
    return hash == 0;
}

template <typename K, typename V>
rhh_bsd<K, V>::rhh_bsd(int cap, float load_factor, K init_key, V init_val):
    load_factor(load_factor)
{
    assert(load_factor >= 0.1 && load_factor <= 1);
    assert(cap && !(cap & (cap - 1)));
    arr = std::vector<elem>(cap, elem(init_key, init_val, 0));
}

template <typename K, typename V>
uint32_t rhh_bsd<K, V>::hash(K const &key) const
{
    return (key % arr.capacity()) | 0x80000000;
}

template <typename K, typename V>
uint32_t rhh_bsd<K, V>::desired_pos(uint32_t const &hash) const
{
    return hash & 0x7FFFFFFF;
}

template <typename K, typename V>
uint32_t rhh_bsd<K, V>::cyclic_dist(uint32_t const &origin, uint32_t const &cursor) const
{
    return (cursor + arr.capacity() - origin) % arr.capacity();
}

template <typename K, typename V>
void rhh_bsd<K, V>::do_insert(
    uint32_t const &pos,
    uint32_t const &hash,
    K const &key,
    V const &val)
{
    arr[pos].key = key;
    arr[pos].val = val;
    arr[pos].hash = hash;
    ++size;
}

template <typename K, typename V>
void rhh_bsd<K, V>::insert_elem(K key, V val)
{
    if (size >= arr.capacity() * load_factor)
	double_capacity();
    uint32_t h = hash(key);
    uint32_t origin = desired_pos(h);
    uint32_t pos = origin;
    uint32_t existing = 0;
    for (;;) {
	if (arr[pos].empty()) {
	    do_insert(pos, h, key, val);
	    return;
	}
	existing = cyclic_dist(desired_pos(arr[pos].hash), pos);
	if (cyclic_dist(origin, pos) > existing) {
	    std::swap(key, arr[pos].key);
	    std::swap(val, arr[pos].val);
	    std::swap(h, arr[pos].hash);
	}
	pos = (pos + 1) % arr.capacity();
    }
}

template <typename K, typename V>
bool rhh_bsd<K, V>::delete_elem(K const &key)
{
    uint32_t i;
    bool found = find_pos(key, i);
    if (found) {
	int next;
	for (;;) {
	    next = (i + 1) % arr.capacity();
	    if (arr[next].hash == 0 ||
		desired_pos(arr[next].hash) == next) {
		break;
	    } else {
		arr[i] = arr[next];
	    }
	}
	--size;
    }
    return found;
}

template <typename K, typename V>
bool rhh_bsd<K, V>::get_elem(K const &key, V &val) const
{
    uint32_t i;
    bool found = find_pos(key, i);
    if (found)
	val = arr[i].val;
    return found;
}

template <typename K, typename V>
uint32_t rhh_bsd<K, V>::get_capacity() const
{
    return this->arr.capacity();
}

template <typename K, typename V>
uint32_t rhh_bsd<K, V>::get_size() const
{
    return this->size;
}

template <typename K, typename V>
bool rhh_bsd<K, V>::find_pos(K const &key, uint32_t &pos) const
{
    uint32_t origin = desired_pos(hash(key));
    pos = origin;
    for (uint32_t d = 0; d < arr.capacity(); ++d) {
	pos = (origin + d) % arr.capacity();
	if (arr[pos].empty())
	    break;
	else if (arr[pos].key == key)
	    return true;
	else if (d > cyclic_dist(desired_pos(arr[pos].hash), pos))
	    break;
    }
    return false;
}

template <typename K, typename V>
void rhh_bsd<K, V>::double_capacity()
{
    std::vector<elem> old_arr(arr);
    arr = std::vector<elem>(old_arr.capacity() * 2, elem());
    size = 0;
    for (uint32_t i = 0; i < old_arr.capacity(); ++i) {
	if (!old_arr[i].empty())
	    insert_elem(old_arr[i].key, old_arr[i].val);
    }
    return;
}

template <typename K, typename V>
int32_t rhh_bsd<K, V>::avg_cyclic_dist() const
{
    long long sum = 0;
    for(int pos = 0; pos < arr.capacity(); ++pos)
    {
	if (!arr[pos].empty())
	    sum += cyclic_dist(desired_pos(arr[pos].hash), pos);
    }
    return static_cast<int32_t>(sum / arr.capacity());
}

#endif
