/*
  hashset.hpp
  S. Edward Dolan
  Friday, June 16 2023
*/

#ifndef HASHSET_HPP_INCLUDED
#define HASHSET_HPP_INCLUDED

/*
  Can be a function:
  (#{1 2 3} 3)            => 3
  (#{1 2 3} 4)            => nil
  (#{1 2 3} 4 :not-found) => :not-found
 */
struct Hashset : Fn,
                 ISeqable, ICollection, ISet, IMeta {
    static Hashset* create();
    static Hashset* create(vecobj_t);
    const hashset_t& impl() const;
    //
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Hashset* copy();
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
    Hashset* withMeta(Hashmap*);
protected:
    hashset_t _impl;
    Hashmap* _meta;
    void createMethods();
    Hashset();
};
DEF_CASTER(Hashset)

#endif // HASHSET_HPP_INCLUDED
