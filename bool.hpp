/*
  bool.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef BOOL_HPP_INCLUDED
#define BOOL_HPP_INCLUDED

struct Bool : ISortable {
    static Bool* create(bool val);
    std::string toString();
    size_t getHash();
    Bool* copy() { return this; }
    //
    bool less(Obj*);
protected:
    static Bool* TRUE;
    static Bool* FALSE;
    bool _val;
    size_t _hash;
    Bool(bool);
};
DEF_CASTER(Bool)

#endif // BOOLEAN_HPP_INCLUDED
