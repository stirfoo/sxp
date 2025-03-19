/*
  treeset.hpp
  S. Edward Dolan
  Sunday, June 25 2023
*/

#ifndef TREESET_HPP_INCLUDED
#define TREESET_HPP_INCLUDED

struct Treeset: Fn, ISeqable, ICollection, ISet, IMeta {
    static Treeset* create();
    static Treeset* create(vecobj_t);
    const treeset_t& impl() const;
    // 
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Treeset* copy();
    //
    ISeq* seq();
    //
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);
    //
    ISet* disjoin(Obj*);
    bool contains(Obj*);
    Obj* get(Obj*, Obj* notFound=NIL);
    //
    Hashmap* meta();
    Obj* withMeta(Hashmap*);
protected:
    treeset_t _impl;
    Hashmap* _meta;
    void createMethods();
    Treeset();
};
DEF_CASTER(Treeset)

#endif // TREESET_HPP_INCLUDED
