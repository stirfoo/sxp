/*
  mapentry.hpp
  S. Edward Dolan
  Saturday, April  8 2023
*/

#ifndef MAPENTRY_HPP_INCLUDED
#define MAPENTRY_HPP_INCLUDED

struct MapEntry : ISeqable, IIndexed, ICollection {
    static MapEntry* create(Obj* key, Obj* val);
    // 
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    MapEntry* copy();
    //
    ISeq* seq();
    //
    Obj* nth(int);
    Obj* nth(int, Obj*);
    //
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);
    // 
    Obj* key();
    Obj* val();
protected:    
    Obj* _key;
    Obj* _val;
    MapEntry(Obj*, Obj*);
};
DEF_CASTER(MapEntry)

#endif // MAPENTRY_HPP_INCLUDED
