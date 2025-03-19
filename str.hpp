/*
  str.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef STR_HPP_INCLUDED
#define STR_HPP_INCLUDED

struct String : ISeqable, IIndexed, ICollection, ISortable {
    friend struct WeakRefMap<String>;
    static void shutdown() { _cache.clear(); }
    static String* fetch(const std::string&);  // cached
    static String* create(const std::string&); // not cached
    static void dumpCache();
    const std::string& val() { return _val; }
    
    //
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    String* copy() { return this; }
    //
    ISeq* seq();
    //
    Obj* nth(int);
    Obj* nth(int, Obj*);
    // 
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);    // creates a new String instance
    //
    bool less(Obj*);
protected:    
    std::string _val;
    size_t _hash;
    static WeakRefMap<String> _cache;
    String(const std::string& s);
};
DEF_CASTER(String)

#endif // STR_HPP_INCLUDED
