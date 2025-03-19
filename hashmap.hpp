/*
  hashmap.hpp
  S. Edward Dolan
  Saturday, April  8 2023
*/

#ifndef HASHMAP_HPP_INCLUDED
#define HASHMAP_HPP_INCLUDED

/*
  Can be a function:
  ({:one 1 :two 2} :two)              => 2
  ({:one 1 :two 2} :three)            => nil
  ({:one 1 :two 2} :three :not-found) => :not-found
 */
struct Hashmap : Fn, ISeqable, ICollection, IAssociative, IMeta {
    static Hashmap* create();
    static Hashmap* create(const vecobj_t& v);
    static Hashmap* create(hashmap_t keysvals);
    const hashmap_t& impl() const;
    void clear();
    //
    std::string toString();
    // TODO: a mutable hashmap really should not be hashable, but clojure's
    // DESTRUCTURE depends on it being so. Obj::getHash() is used which hashes
    // this pointer.
    // size_t getHash();
    bool isEqualTo(Obj*);
    Hashmap* copy();
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
    hashmap_t _impl;
    Hashmap* _meta;
    void createMethods();
    Hashmap();
};
DEF_CASTER(Hashmap)

#endif // HASHMAP_HPP_INCLUDED
