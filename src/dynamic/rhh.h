#ifndef RHH_H
#define RHH_H

#include <cassert>
#include <cmath>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/* Robin Hood Hash Map Implementation */

template <typename K, typename V>
class rhh_elem {
public:
        K key;
        V val;
        uint32_t hash;
        rhh_elem(K k, V v, uint32_t h):
                key(k),
                val(v),
                hash(h) {}
        rhh_elem(K k, V v):
                rhh_elem(k, v, 0) {}
        rhh_elem():
                rhh_elem(K{}, V{}, 0) {}
        inline void mark_deleted();
        inline bool empty() const;
        inline bool deleted() const;
        std::string to_string();
};

template <typename K, typename V>
void rhh_elem<K, V>::mark_deleted()
{
        hash |= 0x80000000;
}

template <typename K, typename V>
bool rhh_elem<K, V>::empty() const
{
        return hash == 0;
}

template <typename K, typename V>
bool rhh_elem<K, V>::deleted() const
{
        return hash >> 31 == 1;
}

template <typename K, typename V>
std::string rhh_elem<K, V>::to_string()
{
        std::ostringstream os;
        os << *this;
        return os.str();
}

template <typename K, typename V>
class rhh {
public:
        std::vector<rhh_elem<K, V>> arr; // This shouldn't be public. But screw it.
        rhh(int cap, float load_factor, K init_key, V init_val);
        rhh(int cap, float load_factor, V init_val):
                rhh(cap, load_factor, K{}, init_val) {}
        rhh(int cap, float load_factor):
                rhh(cap, load_factor, V{}) {}
        rhh(int cap):
                rhh(cap, 0.9) {}
        rhh():
                rhh(pow(2, 20)) {}
        rhh(rhh&) = delete;
        void insert_elem(K key, V val);
        bool delete_elem(K const &key);
        bool get_elem(K const &key, V &val) const;
        inline uint32_t get_capacity() const;
        inline uint32_t get_size() const;
        int32_t avg_probe_dist() const;
        std::string to_string() const;
protected:
        inline uint32_t hash(K const &key) const;
        inline uint32_t desired_pos(uint32_t const &hash) const;
        inline uint32_t probe_dist(
                uint32_t const &origin,
                uint32_t const &cursor) const;
        inline void do_insert(
                uint32_t const &pos,
                uint32_t const &hash,
                K const &k,
                V const &v);
        bool find_elem(K const &key, uint32_t &i) const;
        void double_capacity();
        float load_factor;
        uint32_t size = 0;
};

template <typename K, typename V>
rhh<K, V>::rhh(int cap, float load_factor, K init_key, V init_val):
        load_factor(load_factor)
{
        assert(load_factor >= 0.1 && load_factor <= 1);
        assert(cap && !(cap & (cap - 1)));
        rhh_elem<K, V> init(init_key, init_val, 0);
        arr = std::vector<rhh_elem<K, V>>(cap, init);
}

template <typename K, typename V>
uint32_t rhh<K, V>::hash(K const &key) const
{
        return ((key % arr.capacity()) & 0x3FFFFFFF) | 0x40000000;
}

template <typename K, typename V>
uint32_t rhh<K, V>::desired_pos(uint32_t const &hash) const
{
        return hash & 0x3FFFFFFF;
}

template <typename K, typename V>
uint32_t rhh<K, V>::probe_dist(uint32_t const &origin, uint32_t const &cursor) const
{
        return (cursor + arr.capacity() - origin) % arr.capacity();
}

template <typename K, typename V>
void rhh<K, V>::do_insert(
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
void rhh<K, V>::insert_elem(K key, V val)
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
                if (arr[pos].key == key) {
                        arr[pos].val = val;
                        if (arr[pos].deleted())
                                arr[pos].hash = h;
                        return;
                }
                existing = probe_dist(desired_pos(arr[pos].hash), pos);
                if (probe_dist(origin, pos) > existing) {
                        if (arr[pos].deleted()) {
                                do_insert(pos, h, key, val);
                                return;
                        }
                        std::swap(key, arr[pos].key);
                        std::swap(val, arr[pos].val);
                        std::swap(h, arr[pos].hash);
                }
                pos = (pos + 1) % arr.capacity();
        }
}

template <typename K, typename V>
bool rhh<K, V>::delete_elem(K const &key)
{
        uint32_t i;
        bool found = find_elem(key, i);
        if (found) {
                arr[i].mark_deleted();
                --size;
        }
        return found;
}

template <typename K, typename V>
bool rhh<K, V>::get_elem(K const &key, V &val) const
{
        uint32_t i;
        bool found = find_elem(key, i);
        if (found)
                val = arr[i].val;
        return found;
}

template <typename K, typename V>
uint32_t rhh<K, V>::get_capacity() const
{
        return this->arr.capacity();
}

template <typename K, typename V>
uint32_t rhh<K, V>::get_size() const
{
        return this->size;
}

template <typename K, typename V>
std::string rhh<K, V>::to_string() const
{
        std::ostringstream os;
        os << *this;
        return os.str();
}

template <typename K, typename V>
bool rhh<K, V>::find_elem(K const &key, uint32_t &pos) const
{
        uint32_t origin = desired_pos(hash(key));
        pos = origin;
        for (uint32_t d = 0; d < arr.capacity(); ++d) {
                pos = (origin + d) % arr.capacity();
                if (arr[pos].empty())
                        break;
                else if (arr[pos].key == key && !arr[pos].deleted())
                        return true;
                else if (d > probe_dist(desired_pos(arr[pos].hash), pos))
                        break;
        }
        return false;
}

template <typename K, typename V>
void rhh<K, V>::double_capacity()
{
        std::vector<rhh_elem<K, V>> old_arr(arr);
        arr = std::vector<rhh_elem<K, V>>(
                old_arr.capacity() * 2,
                rhh_elem<K, V>());
        size = 0;
        for (uint32_t i = 0; i < old_arr.capacity(); ++i) {
                if (!old_arr[i].empty() && !old_arr[i].deleted())
                        insert_elem(old_arr[i].key, old_arr[i].val);
        }
        return;
}

template <typename K, typename V>
int32_t rhh<K, V>::avg_probe_dist() const
{
        long long sum = 0;
        for(int pos = 0; pos < arr.capacity(); ++pos)
        {
                if (!arr[pos].empty() && !arr[pos].deleted())
                        sum += probe_dist(desired_pos(arr[pos].hash), pos);
        }
        return static_cast<int32_t>(sum / arr.capacity());
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream &out, rhh_elem<K, V> const &el)
{
        out << "(key=" << el.key << ",";
        out << "val=" << el.val << ",";
        out << "hash=" << el.hash << ",";
        return out;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream &out, rhh<K, V> const &rhh)
{
        out << "(";
        for (uint32_t i = 0; i < rhh.get_capacity(); i++) {
                out << "arr[" << i << "]=";
                out << rhh.arr[i];
                if (i < rhh.get_capacity() - 1)
                        out << ",\n";
        }
        out << ")";
        return out; 
}

#endif
