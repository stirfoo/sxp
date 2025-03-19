/*
  obj.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef OBJ_HPP_INCLUDED
#define OBJ_HPP_INCLUDED

// If t = Foo, emit:
// static inline Foo* pFoo(Obj*) {...}
// static inline Foo* cpFoo(Obj*) {...}
#define DEF_CASTER(t)                                                   \
    static inline t* p##t(Obj* obj) {                                   \
        return dynamic_cast<t*>(obj);                                   \
    }                                                                   \
    static inline t* cp##t(Obj* obj) {                                  \
        if (t* p = dynamic_cast<t*>(obj)) return p;                     \
        throw SxCastError(obj ? obj->typeName() : "nil", #t);           \
    }

/*
  The fundamental sxp object. nil is nullptr.
 */
struct Obj : public gc {
    static Obj* create();
    virtual std::string typeName() const;
    virtual std::string toString();
    virtual size_t getHash();
    virtual bool isEqualTo(Obj*);
    virtual Obj* copy();
protected:    
    std::string _typeName;
    Obj() : _typeName("SxObj") {}
};

#define NIL nullptr

// std::equal predicate used by Vector::isEqualTo()
inline bool objEqual(Obj* o1, Obj* o2) {
    if (!o1)
        return !o2;
    return o1->isEqualTo(o2);
}

struct ObjEqFntr {
    size_t operator()(Obj* o1, Obj* o2) const {
        if (o1 == NIL)
            return o2 == NIL;
        return o1->isEqualTo(o2);
    }
};

struct ObjHashFntr {
    size_t operator()(Obj* obj) const {
        if (obj == NIL)
            return 0;
        return obj->getHash();
    }
};

struct ObjLessFntr { bool operator()(Obj*, Obj*) const; };

// sxp vector impl
typedef std::vector<Obj*,
                    gc_allocator<Obj*>> vecobj_t;

// sxp hashmap impl
typedef std::unordered_map<Obj*,
                           Obj*,
                           ObjHashFntr,
                           ObjEqFntr,
                           gc_allocator<std::pair<Obj* const,
                                                  Obj*>>> hashmap_t;

// sxp hashset impl
typedef std::unordered_set<Obj*,
                           ObjHashFntr,
                           ObjEqFntr,
                           gc_allocator<Obj*>> hashset_t;

// sxp treemap impl
typedef std::map<Obj*,
                 Obj*,
                 ObjLessFntr,
                 gc_allocator<std::pair<Obj* const,
                                        Obj*>>> treemap_t;

// sxp treeset impl
typedef std::set<Obj*,
                 ObjLessFntr,
                 gc_allocator<Obj*>> treeset_t;

// bytecode container
typedef std::vector<uint8_t> vecu8_t;

// for shitty shitty bang bang c++ formatting
#define HEX2(x) std::setw(2) << std::hex << std::setfill('0') << (x)
#define HEX4(x) std::setw(4) << std::hex << std::setfill('0') << (x)

#endif // OBJ_HPP_INCLUDED
