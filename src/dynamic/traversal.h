#ifndef TRAVERSAL_H_
#define TRAVERSAL_H_

#include "types.h"
#include "adListShared.h"
#include "stinger.h"
#include "darhh.h"
#include "adListChunked.h"

#include "topDataStruc.h"

template <typename T>
class neighborhood;

template <typename T> 
class neighborhood_iter {    
public:
    neighborhood_iter(T* ds, NodeID n, bool in_neigh);
    bool operator!=(neighborhood_iter& it);
    neighborhood_iter& operator++();
    neighborhood_iter& operator++(int);
    NodeID operator*();
    Weight extractWeight();
};

template<typename U>
class neighborhood_iter<adList<U>> {    
    friend class neighborhood<adList<U>>;
private:      
    adList<U>* ds;      
    NodeID node;
    bool in_neigh;
    U* cursor;
public:
    neighborhood_iter(adList<U>* _ds, NodeID _n, bool _in_neigh): 
	ds(_ds), node(_n), in_neigh(_in_neigh){        
	if(in_neigh){        
	    bool empty = ds->in_neighbors[node].empty(); 
	    cursor = empty? 0 : &(ds->in_neighbors[node][0]);        
	}
	else{        
	    bool empty = ds->out_neighbors[node].empty(); 
	    cursor = empty? 0 : &(ds->out_neighbors[node][0]);
	}
    }

    bool operator!=(const neighborhood_iter<adList<U>>& it){
        return cursor != it.cursor;
    }

    neighborhood_iter& operator++(){
        if(in_neigh){
            int size_in_neigh = ds->in_neighbors[node].size();
            if(cursor == &(ds->in_neighbors[node][size_in_neigh-1])){
                cursor = nullptr; 
            }
            else cursor = cursor + 1;
        }else{
            int size_out_neigh = ds->out_neighbors[node].size();
            if(cursor == &(ds->out_neighbors[node][size_out_neigh-1])){
                cursor = nullptr;
            }else cursor = cursor + 1;
        }

        return *this;	      
    }

    neighborhood_iter& operator++(int){
         if(in_neigh){
             int size_in_neigh = ds->in_neighbors[node].size();
             if(cursor == &(ds->in_neighbors[node][size_in_neigh-1])){
                 cursor = nullptr; 
             }
             else cursor = cursor + 1;
         }else{
             int size_out_neigh = ds->out_neighbors[node].size();
             if(cursor == &(ds->out_neighbors[node][size_out_neigh-1])){
                 cursor = nullptr;
             }else cursor = cursor + 1;
         }

         return *this;		
    }

    NodeID operator*() {
	return cursor->getNodeID();
    }

    Weight extractWeight(){
	return cursor->getWeight();
    }
};

template <typename U>
class neighborhood_iter<adListShared<U>> {    
    friend class neighborhood<adListShared<U>>;
private:      
    adListShared<U>* ds;      
    NodeID node;
    bool in_neigh;
    U* cursor;
public:
    neighborhood_iter(adListShared<U>* _ds, NodeID _n, bool _in_neigh): 
	ds(_ds), node(_n), in_neigh(_in_neigh){        
	if(in_neigh){        
	    bool empty = ds->in_neighbors[node].empty(); 
	    cursor = empty? 0 : &(ds->in_neighbors[node][0]);        
	}
	else{        
	    bool empty = ds->out_neighbors[node].empty(); 
	    cursor = empty? 0 : &(ds->out_neighbors[node][0]);
	}
    }

    bool operator!=(const neighborhood_iter<adListShared<U>>& it){
        return cursor != it.cursor;
    }

    neighborhood_iter& operator++(){
        if(in_neigh){
            int size_in_neigh = ds->in_neighbors[node].size();
            if(cursor == &(ds->in_neighbors[node][size_in_neigh-1])){
                cursor = nullptr; 
            }
            else cursor = cursor + 1;
        }else{
            int size_out_neigh = ds->out_neighbors[node].size();
            if(cursor == &(ds->out_neighbors[node][size_out_neigh-1])){
                cursor = nullptr;
            }else cursor = cursor + 1;
        }

        return *this;	      
    }

    neighborhood_iter& operator++(int){
         if(in_neigh){
             int size_in_neigh = ds->in_neighbors[node].size();
             if(cursor == &(ds->in_neighbors[node][size_in_neigh-1])){
                 cursor = nullptr; 
             }
             else cursor = cursor + 1;
         }else{
             int size_out_neigh = ds->out_neighbors[node].size();
             if(cursor == &(ds->out_neighbors[node][size_out_neigh-1])){
                 cursor = nullptr;
             }else cursor = cursor + 1;
         }

         return *this;		
    }

    NodeID operator*() {
	return cursor->getNodeID();
    }

    Weight extractWeight(){
	return cursor->getWeight();
    }
};

//specialization for stinger

template <>
class neighborhood_iter<stinger> {    
    friend class neighborhood<stinger>;

    private:      
      stinger* ds;      
      NodeID node;
      bool in_neigh;
      stinger_edge* cursor;
      stinger_eb* curr_eb;
      stinger_vertex* sv;
      int cursor_index;

    public:
      neighborhood_iter(stinger* _ds, NodeID _n, bool _in_neigh)
      :ds(_ds), node(_n), in_neigh(_in_neigh)
      {
          sv = &(ds->vertices[node]);

          if(in_neigh){              
              bool empty = (sv->in_neighbors->numEdges == 0); 
              cursor = empty? 0 : &(sv->in_neighbors->edges[0]);
              curr_eb = sv->in_neighbors;
              if(!empty) cursor_index = 0;
          }

          else{
              bool empty = (sv->out_neighbors->numEdges == 0); 
              cursor = empty? 0 : &(sv->out_neighbors->edges[0]);
              curr_eb = sv->out_neighbors;
              if(!empty) cursor_index = 0;
          }
      }

      bool operator!=(const neighborhood_iter<stinger>& it){
          return cursor != it.cursor;
      }

      neighborhood_iter& operator++(){
          // just increment by 1 if we are in an edgeblock and more edges left           
          if(cursor_index < (curr_eb->numEdges-1)){
              cursor = cursor + 1;
              cursor_index++;
          }

          else if(cursor_index == (curr_eb->numEdges-1)){
              // We are done with current edgeblock 
              if(curr_eb->next != nullptr){                  
                  // move to next one, if there is one
                  curr_eb = curr_eb->next;
                  cursor = &(curr_eb->edges[0]);
                  cursor_index = 0;
              }else{
                  // there is no further, end of traversal
                  cursor = nullptr;
              }              
          }        

          return *this;          
      }

      neighborhood_iter& operator++(int){
          // just increment by 1 if we are in an edgeblock and more edges left           
          if(cursor_index < (curr_eb->numEdges-1)){              
              cursor = cursor + 1;
              cursor_index++;
          }

          else if(cursor_index == (curr_eb->numEdges-1)){
              // We are done with current edgeblock 
              if(curr_eb->next != nullptr){                  
                  // move to next one, if there is one
                  curr_eb = curr_eb->next;
                  cursor = &(curr_eb->edges[0]);
                  cursor_index = 0;
              }else{                  
                  // there is no further, end of traversal
                  cursor = nullptr;
              }              
          }        

          return *this; 
      }      

      NodeID operator*(){
          assert(cursor->neighbor != -1);
          return cursor->neighbor;
      }

      Weight extractWeight(){
          return cursor->weight;
      }
};

// // ------------------------------adList_chunk----------------------------------
template <typename U>
class neighborhood_iter<adListChunked<U>> {    
    friend class neighborhood<adListChunked<U>>;
private:      
    adListChunked<U>* ds;      
    NodeID node;
    bool in_neigh;
    U* cursor;

    int64_t part_idx; 
    int64_t sub_idx;
    
public:
    neighborhood_iter(adListChunked<U>* _ds, NodeID _n, bool _in_neigh): 
	ds(_ds), node(_n), in_neigh(_in_neigh){   
        // part_idx = _n % (ds->num_partitions); 
        // sub_idx = (int) _n/(ds->num_partitions);
        part_idx = ds->pt_hash(_n); 
        sub_idx =  ds->hash_within_chunk(_n);
	    if(in_neigh){      
	        bool empty = ds->in[part_idx]->partAdList->neighbors[sub_idx].empty(); 
    	    cursor = empty? nullptr : &(ds->in[part_idx]->partAdList->neighbors[sub_idx][0]);
	    } 
        else{      
	        bool empty = ds->out[part_idx]->partAdList->neighbors[sub_idx].empty(); 
	        cursor = empty? nullptr : &(ds->out[part_idx]->partAdList->neighbors[sub_idx][0]);
	    }
    }

    bool operator!=(const neighborhood_iter<adListChunked<U>>& it){
        return cursor != it.cursor;
    }

    neighborhood_iter& operator++(){
        if(in_neigh){
            int size_in_neigh = ds->in[part_idx]->partAdList->neighbors[sub_idx].size();
            if(cursor == &(ds->in[part_idx]->partAdList->neighbors[sub_idx][size_in_neigh-1]))
                cursor = nullptr; 
            else
                cursor = cursor + 1;
        }
        else{
            int size_out_neigh = ds->out[part_idx]->partAdList->neighbors[sub_idx].size();
            if(cursor == &(ds->out[part_idx]->partAdList->neighbors[sub_idx][size_out_neigh-1]))
                cursor = nullptr;
            else 
                cursor = cursor + 1;
        }

        return *this;	      
    }

    neighborhood_iter& operator++(int){
        if(in_neigh){
            int size_in_neigh = ds->in[part_idx]->partAdList->neighbors[sub_idx].size();
            if(cursor == &(ds->in[part_idx]->partAdList->neighbors[sub_idx][size_in_neigh-1]))
                cursor = nullptr; 
            else
                cursor = cursor + 1;
        }
        else{
            int size_out_neigh = ds->out[part_idx]->partAdList->neighbors[sub_idx].size();
            if(cursor == &(ds->out[part_idx]->partAdList->neighbors[sub_idx][size_out_neigh-1]))
                cursor = nullptr;
            else 
                cursor = cursor + 1;
        }

         return *this;		
    }

    NodeID operator*() {
	    return cursor->getNodeID();
    }

    Weight extractWeight(){
	    return cursor->getWeight();
    }
};

template <typename U>
class neighborhood_iter<darhh<U>> {    
    friend class neighborhood<darhh<U>>;
private:
    hd_rhh<U> *hd;
    ld_rhh<U> *ld;
    typename hd_rhh<U>::iter hd_iter;
    typename ld_rhh<U>::iter ld_iter;
    bool low_degree;
public:
    inline neighborhood_iter& operator=(neighborhood_iter const &it);
    inline bool operator!=(neighborhood_iter const &it);
    neighborhood_iter& operator++();
    neighborhood_iter& operator++(int);
    inline NodeID operator*();
    inline Weight extractWeight();
    void set_begin(darhh<U> *ds, NodeID n, bool in);
    void set_end();
};

template <typename U>
void neighborhood_iter<darhh<U>>::set_begin(darhh<U> *ds, NodeID src, bool in)
{
    if (in) {
	ld = ds->in[ds->pt_hash(src)]->ld;
	hd = ds->in[ds->pt_hash(src)]->hd;
    } else {	
	ld = ds->out[ds->pt_hash(src)]->ld;
	hd = ds->out[ds->pt_hash(src)]->hd;
    }
    low_degree = ld->get_degree(src);
    if (low_degree)
	ld_iter = ld->begin(src);
    else
	hd_iter = hd->begin(src);
}

template <typename U>
void neighborhood_iter<darhh<U>>::set_end()
{
    ld_iter.cursor = nullptr;
    hd_iter.cursor = nullptr;
}

template <typename U>
neighborhood_iter<darhh<U>>&
neighborhood_iter<darhh<U>>::operator=(neighborhood_iter const &other)
{
    hd = other.hd;
    ld = other.ld;
    hd_iter = other.hd_iter;
    ld_iter = other.ld_iter;
    low_degree = other.low_degree;
    return *this;
}

template <typename U>
bool neighborhood_iter<darhh<U>>::operator!=(neighborhood_iter const &it)
{
    if (low_degree) 
	return ld_iter != it.ld_iter;
    else
	return hd_iter != it.hd_iter;
}

template <typename U>
neighborhood_iter<darhh<U>>& neighborhood_iter<darhh<U>>::operator++()
{
    if (low_degree)
        ++ld_iter;
    else
	++hd_iter;
    return *this;
}

template <typename U>
neighborhood_iter<darhh<U>>& neighborhood_iter<darhh<U>>::operator++(int)
{
    if (low_degree)
        ++ld_iter;
    else
	++hd_iter;
    return *this;
}

template <typename U>
NodeID neighborhood_iter<darhh<U>>::operator*()
{
    if (low_degree)
        return ld_iter.cursor->getNodeID();
    else
	return hd_iter.cursor->getNodeID();   
}

template <typename U>
Weight neighborhood_iter<darhh<U>>::extractWeight()
{
    if (low_degree)
	return ld_iter.cursor->getWeight();
    else
	return hd_iter.cursor->getWeight();
}

template <typename T>
class neighborhood {
private:
    T* ds;
    NodeID node;
    bool in_neigh; 
public:
    neighborhood(NodeID _node, T* _ds, bool _in_neigh):
	ds(_ds),
	node(_node),
	in_neigh(_in_neigh) {}
    neighborhood_iter<T> begin() {
	return neighborhood_iter<T>(ds, node, in_neigh);
    } 
    neighborhood_iter<T> end() {
        neighborhood_iter<T> n = neighborhood_iter<T>(ds, node, in_neigh);
        n.cursor = nullptr;
        return n;
    }
};

template <typename U>
class neighborhood<darhh<U>> {    
private:
    using iter = neighborhood_iter<darhh<U>>;
    NodeID src;
    darhh<U> *ds;
    bool in;
public:
    neighborhood(NodeID src, darhh<U> *ds, bool in): src(src), ds(ds), in(in) {}
    iter begin() {
	iter it;
	it.set_begin(ds, src, in);
	return it;
    } 
    iter end() {
        iter it;
	it.set_end();
        return it;
    }
};

template<typename T>
neighborhood<T> in_neigh(NodeID n, T* ds)
{
    if(ds->directed)
	return neighborhood<T>(n, ds, true);         
    else
	return neighborhood<T>(n, ds, false); 
}

template<typename T>
neighborhood<T> out_neigh(NodeID n, T* ds)
{
    return neighborhood<T>(n, ds, false); 
}

#endif // TRAVERSAL_H_
