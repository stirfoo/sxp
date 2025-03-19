/*
  treemap.hpp
  S. Edward Dolan
  Saturday, June 17 2023
*/

#ifndef TREEMAP_HPP_INCLUDED
#define TREEMAP_HPP_INCLUDED

struct Treemap : Fn, ISeqable, ICollection, IAssociative, IMeta {
    static Treemap* create();
    static Treemap* create(const vecobj_t& v);
    static Treemap* create(treemap_t keysvals);
    const treemap_t& impl() const;
    ISeq* rseq();
    void clear();
    //
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Treemap* copy();
    //
    ISeq* seq();
    //
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);
    //
    IAssociative* assoc(Obj*, Obj*);
    IAssociative* dissoc(Obj*);
    bool hasKey(Obj*);
    MapEntry* entryAt(Obj*);
    Obj* valAt(Obj* key, Obj* notFound=NIL);
    //
    Hashmap* meta() { return _meta; }
    Obj* withMeta(Hashmap* m) { _meta = m; return this; }
protected:    
    treemap_t _impl;
    Hashmap* _meta;
    void createMethods();
    Treemap();
};
DEF_CASTER(Treemap)

#endif // TREEMAP_HPP_INCLUDED
