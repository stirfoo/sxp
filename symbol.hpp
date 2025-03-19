/*
  symbol.hpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#ifndef SYMBOL_HPP_INCLUDED
#define SYMBOL_HPP_INCLUDED

struct Namespace;

struct Symbol : IMeta, ISortable {
    friend struct Namespace;
    static Symbol* create(const std::string& maybeQualifiedName);
    static Symbol* create(const std::string& nsName, const std::string& name);
    //
    std::string nsName() { return _nsName; }
    std::string name() { return _name; }
    bool hasNS() { return !_nsName.empty(); }
    // Obj
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    // IMeta
    bool hasMeta();
    Hashmap* meta();
    Symbol* withMeta(Hashmap*);
    // ISortable
    bool less(Obj*);
protected:    
    std::string _nsName;
    std::string _name;
    size_t _hash;
    Hashmap* _meta;
    Symbol(const std::string& nsName, const std::string& name);
};
DEF_CASTER(Symbol)

#endif // SYMBOL_HPP_INCLUDED
