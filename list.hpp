/*
  list.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef LIST_HPP_INCLUDED
#define LIST_HPP_INCLUDED

struct List : ISeqable, ISeq, IIndexed, ICollection, IMeta {
    static List* create();
    static List* create(Obj*);
    static List* create(Obj*, Obj*);
    static List* create(Obj*, Obj*, Obj*);
    static List* create(Obj*, Obj*, Obj*, Obj*);
    static List* create(vecobj_t);
    // 
    static void init();
    void setHead(Obj*);
    // 
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    List* copy();
    //
    Obj* first();
    ISeq* rest();
    ISeq* next();
    ISeq* cons(Obj*);
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
    Hashmap* meta();
    Obj* withMeta(Hashmap*);
protected:
    static Obj* EMPTY_LIST_MARKER;
    Obj* _head;
    ISeq* _tail;
    Hashmap* _meta;
    List();
    List(Obj*);
    List(Obj*, Obj*);
    List(Obj*, Obj*, Obj*);
    List(Obj*, Obj*, Obj*, Obj*);
};
DEF_CASTER(List)

#endif // LIST_HPP_INCLUDED
