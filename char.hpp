/*
  char.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef CHAR_HPP_INCLUDED
#define CHAR_HPP_INCLUDED

struct Character : ISortable {
    static void init();
    static void shutdown();
    static Character* fetch(int);
    int val();
    // 
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Character* copy() { return this; }
    //
    bool less(Obj*);
protected:
    int _val;
    size_t _hash;
    static std::vector<Character*, gc_allocator<Character*>> _cache;
    Character();
    Character(int c);
};
DEF_CASTER(Character)

#endif // CHAR_HPP_INCLUDED
