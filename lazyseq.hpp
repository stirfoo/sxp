/*
  lazyseq.hpp
  S. Edward Dolan
  Saturday, June 17 2023
*/

#ifndef LAZYSEQ_HPP_INCLUDED
#define LAZYSEQ_HPP_INCLUDED

struct LazySeq : ISeq, ISeqable, ICollection {
    static LazySeq* create(Obj*);
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    // 
    ISeq* seq();
    // 
    Obj* first();
    ISeq* rest();
    ISeq* next();
    ISeq* cons(Obj*);
    // 
    int count();
    bool isEmpty();
    ICollection* conj(Obj*);
protected:
    Obj* _fn;                   // producer of _sv
    Obj* _sv;                   // result of _fn call
    ISeq* _s;                   // cached result of _fn call
    Obj* sval();
    LazySeq(Obj*);
};
DEF_CASTER(LazySeq)

#endif // LAZYSEQ_HPP_INCLUDED
