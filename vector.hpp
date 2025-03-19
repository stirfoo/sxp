/*
  vector.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef VECTOR_HPP_INCLUDED
#define VECTOR_HPP_INCLUDED

/*
 Can be a function, 1st arg is index:

 ([1 2 3] 1)             => 2
 ([1 2 3] 42)            => throw
 ([1 2 3] 42 :not-found) => :not-found
 */
struct Vector : Fn, ISeqable, IIndexed, ICollection, IMeta {
    static Vector* create();
    static Vector* create(const vecobj_t&);
    const vecobj_t& impl() const;
    static vecu8_t _bc;
    //
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Vector* copy() { return create(_impl); }
    // ISeqable
    ISeq* seq();
    // IIndexed
    Obj* nth(int);
    Obj* nth(int, Obj*);
    // ICollection
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);
    // 
    Hashmap* meta() { return _meta; }
    Obj* withMeta(Hashmap* m) { _meta = m; return this; }
protected:    
    vecobj_t _impl;
    Hashmap* _meta;
    void createMethods();
    Vector();
    Vector(const vecobj_t&);
};
DEF_CASTER(Vector)

#endif // VECTOR_HPP_INCLUDED
