#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 10>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using const_iterator = Iterator;
  using iterator = const_iterator; 
  using key_equal = std::equal_to<key_type>;                       // Hashing
  using hasher = std::hash<key_type>;                              // Hashing

private:
  struct Entry;
  struct Entry {
    key_type key{};
    Entry *next;
    Entry(const value_type &key, Entry *next = nullptr): key(key), next(next) {}
  };

  size_type elem_sz;    
  size_type table_sz;   
  Entry** table;
  size_type coll;

  size_type hashwert(const key_type& k, const size_type& sz) const {           
    hasher hash;
    return hash(k) % sz;
  }

  void rehash() {
    coll = 0;                                       
    Entry** newtable = new Entry* [(4*table_sz)+1];

    for (size_type i{0}; i <= 4*table_sz; i++) newtable[i] = nullptr; 

    for(size_type i{0}; i < table_sz; i++) {
      if (table[i] != nullptr) {
        Entry* temp{table[i]};
        Entry* temp2{nullptr};
        do{
          temp2 = temp->next;
          size_type pos = hashwert(temp->key, 4*table_sz);
          if (newtable[pos] == nullptr) newtable[pos] = new Entry{temp->key};
          else {
            coll++;
            Entry* newtemp{newtable[pos]};
            while(1){
              if(newtemp->next == nullptr) {newtemp->next =  new Entry{temp->key}; break;}
              else newtemp = newtemp->next;
            }
          }
          delete temp;
          temp = temp2;
        }
        while(temp != nullptr);
      } 
    }

    delete [] table;
    table_sz *= 4;
    table = newtable;
  }


public:

  ADS_set(): elem_sz(0), table_sz(N), coll(0) {       
    table = new Entry* [table_sz+1]; 
    for (size_type i{0}; i <= table_sz; i++) table[i] = nullptr; 
  }        

  ADS_set(std::initializer_list<key_type> ilist): ADS_set{} {       
    insert(ilist);
  }       

  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} {        
    insert(first, last);
  }      

  ADS_set(const ADS_set &other) : ADS_set{other.begin(), other.end()} {}
  
  ~ADS_set(){
    for(size_type i{0}; i < table_sz; i++) {
  	  Entry* temp1{table[i]};
      while(temp1 != nullptr){
        Entry* temp2 = temp1->next;
        delete temp1;
        temp1 = nullptr;
        temp1 = temp2;
      }
    }
    delete [] table;
  }
  
  ADS_set &operator=(const ADS_set &other){
    if (this == &other) return *this;
    ADS_set temp{other};
    swap(temp);
    return *this;
  }
  
  ADS_set &operator=(std::initializer_list<key_type> ilist){
    ADS_set temp{ilist};
    swap(temp);
    return *this;
  }

  size_type size() const {return elem_sz;}        
  bool empty() const {return elem_sz==0;}       

  void insert(std::initializer_list<key_type> ilist) {       
      insert(std::begin(ilist), std::end(ilist));
  }

  std::pair<iterator,bool> insert(const key_type &key) {
    auto found {find(key)};
    if (found != end()) return {found, false};
    insert({key});
    auto pos = hashwert(key, table_sz);
    Iterator it{table+pos, table_sz-pos};     
    while(!key_equal{}(*it, key)) {
      it++;
    }
    return {it, true};
  }

  template<typename InputIt> void insert(InputIt first, InputIt last) {   
    for (auto it = first; it != last; ++it) {
      size_type pos = hashwert(*it, table_sz);
      if (table[pos] == nullptr) { 
        table[pos] = new Entry(*it);
        elem_sz++;
        continue;
      }
      else{
        bool vorhanden{false};

        if(table[pos] != nullptr) {
          Entry* temp{table[pos]};
          do {
            if ( key_equal{}(temp->key, *it) ) {vorhanden = true; break;}
            else temp = temp->next;
          }
          while(temp != nullptr);
        }
        if (!vorhanden) {
          elem_sz++;        
          coll++;  
          if (coll * 100 / table_sz >= 40) {
            rehash();
            pos = hashwert(*it, table_sz);
          }
          Entry *newkey = new Entry(*it);
          newkey->next = table[pos];   
          table[pos] = newkey;
        }
      }
    }         
  }

  void clear() {
    ADS_set temp;
    swap(temp);
  }

  size_type erase(const key_type &key) {
    if (!count(key)) return 0;
    auto pos = hashwert(key, table_sz);
    Entry *temp = table[pos],  *vor = nullptr;
    while (temp != nullptr && !key_equal{}(temp->key, key)) {
      vor = temp;     
      temp = temp->next; 
    }   
    if (vor == nullptr) table[pos] = temp->next;
    else vor->next = temp->next;
    delete temp; 
    --elem_sz;
    return 1;
  }

  size_type count(const key_type &key) const {                         
      auto pos = hashwert(key, table_sz);
      if (elem_sz && table[pos] != nullptr) {
        Entry* temp{table[pos]}; 
        do { 
            if (key_equal{}(temp->key, key)) return 1;
            temp = temp->next;
          }
          while(temp != nullptr);
      }
      return 0; 
  }


  iterator find(const key_type &key) const {
    if (!count(key)) return end();             
    auto pos = hashwert(key, table_sz);
    Iterator it{table+pos, table_sz-pos};     
    while(!key_equal{}(*it, key)) {
      it++;
    }
    return it;
  }

  void swap(ADS_set &other) {      
    std::swap(table, other.table);
    std::swap(elem_sz, other.elem_sz);
    std::swap(table_sz, other.table_sz);
    std::swap(coll, other.coll);
  }

  void dump(std::ostream &o = std::cerr) const {
    for(size_type i{0}; i<table_sz; i++) {
      o << "[" << i << "]"; 
      if(table[i] != nullptr) {
        Entry* temp{table[i]};
        do {
          o << " -> " << temp->key;
          temp = temp->next;
        }
        while(temp != nullptr);
      }
      o << '\n';
    }
  }

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
    if (lhs.elem_sz != rhs.elem_sz ) return false;
    for (const auto &val: rhs) if (!lhs.count(val)) return false;
    return true;
  }

  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) {
    return !(lhs==rhs);
  }

  const_iterator begin() const {
    return const_iterator{table, table_sz};
  }

  const_iterator end() const {
    return const_iterator{table+table_sz, 0};
  }
};

  template <typename Key, size_t N>
  class ADS_set<Key,N>::Iterator {
  public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;

  private:
    Entry** pos;
    Entry* e;
    Entry** last; 

  public:
    explicit Iterator(Entry** table, size_type table_sz): pos{table}, e{nullptr}, last{table+table_sz} {
     if (pos != last) e = pos[0];
      while(e == nullptr && pos != last) {
        pos++;
        e = pos[0];
      }
    }
    Iterator(): pos{nullptr},  e{nullptr}, last{nullptr} {}

    reference operator*() const {return e->key;}
    pointer operator->() const {return &(e->key);}   

    Iterator &operator++() {
      if (pos == last) return *this;
      if( e != nullptr && e->next != nullptr) {   
        e = e->next;
        return *this;
      }
      pos++;
      e = pos[0];
      while(e == nullptr && pos != last) {
        pos++;
        e = pos[0];
      }
      return *this;
    }
    
    Iterator operator++(int) {
      auto kopie{*this};
      if (pos == last) return kopie;
      ++*this;
      return kopie;     
    }
    
    friend bool operator==(const Iterator &lhs, const Iterator &rhs) {     
      return (lhs.pos == rhs.pos && lhs.e == rhs.e);
    }

    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {       
      return !(lhs == rhs);
    }  

  };

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); } 

#endif