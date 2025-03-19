/*
  var.hpp
  S. Edward Dolan
  Thursday, May 11 2023
*/

#ifndef VAR_HPP_INCLUDED
#define VAR_HPP_INCLUDED

struct Namespace;

struct Var : IMeta {
    friend struct Namespace;
    static Var* intern(Namespace*, Symbol*, Obj*);
    static Var* create();
    static Var* create(Obj* val);
    static Var* create(Namespace*, Symbol*, Obj*);
    static void pushBindings(Hashmap*);
    static void popBindings();
    std::string toString();
    Namespace* ns() { return _ns; }
    Symbol* sym() { return _sym; }
    Obj* get();
    Obj* set(Obj*);
    Obj* setRoot(Obj*, bool resetMacro=true);
    void pushDyn(Obj*);
    void popDyn();
    bool isDynamic();
    void setDynamic();
    bool isBound();
    bool isDynBound() { return !_dynVals.empty(); }
    bool isMacro();
    bool isPublic();
    // IMeta
    Hashmap* meta() { return _meta; }
    Var* withMeta(Hashmap* m);
protected:
    static vecobj_t _dynVars;
    Namespace* _ns;
    Symbol* _sym;
    Obj* _rootVal;
    vecobj_t _dynVals;
    Hashmap* _meta;
    Var(Namespace*, Symbol*, Obj*);
};
DEF_CASTER(Var)

// RAII for internal use only. Not accessible by the sexp user.
struct DynScope : gc {
    DynScope(Var* var, Obj* obj) : _var(var) {
        var->pushDyn(obj);
    }
    DynScope(hashmap_t varsvals) : _var(nullptr), _varsvals(varsvals) {
        for (auto e : varsvals)
            cpVar(e.first)->pushDyn(e.second);
    }
    ~DynScope() {
        /*
          When a DynScope instance goes out of scope or an exception occurs,
          this destructor is guaranteed to be called, popping the dynamic
          var(s) it pushed.
        */
        if (_var)
            _var->popDyn();
        for (auto e : _varsvals)
            pVar(e.first)->popDyn();
    }
protected:
    Var* _var;
    hashmap_t _varsvals;
};

#endif // VAR_HPP_INCLUDED
