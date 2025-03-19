/*
  namespace.hpp
  S. Edward Dolan
  Thursday, May 11 2023
*/

#ifndef NAMESPACE_HPP_INCLUDED
#define NAMESPACE_HPP_INCLUDED

/*
  A named map where the keys are symbols and the values are vars or constant
  objects.
*/
struct Namespace : Obj {
    static void init();
    static void shutdown();
    static Namespace* find(Symbol*);
    static Namespace* fetch(Symbol*);
    std::string toString();
    Var* intern(Symbol*);
    Symbol* name() { return _name; }
    Hashmap* bindings() { return _bindings; }
    Namespace* lookupAlias(Symbol*);
    Var* findInternedVar(Symbol*);
    Obj* get(Symbol*); // return a Var or a `constant' Obj
    void refer(Namespace* src);
protected:
    static Hashmap* _namespaces;
    Symbol* _name;
    Hashmap* _bindings;
    Hashmap* _aliases;
    Namespace(Symbol* name);
    void warnOrFailOnReplace(Symbol* sym, Obj* curVal, Obj* newVal);
    void reference(Symbol* sym, Obj* val);
};
DEF_CASTER(Namespace)


#endif // NAMESPACE_HPP_INCLUDED
