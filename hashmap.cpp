/*
  hashmap.cpp
  S. Edward Dolan
  Saturday, April  8 2023
*/

#include "sxp.hpp"

// =========================================================================
// static creation

Hashmap* Hashmap::create() {
    return new Hashmap();
}

Hashmap* Hashmap::create(const vecobj_t& v) {
    assert((v.size() % 2) == 0 && "HASHMAP K/V VECTOR MISSING FINAL VALUE");
    Hashmap* m = new Hashmap();
    if (v.size() == 0)
        return m;
    for (auto itr=v.begin(); itr!=v.end(); ++itr) {
        Obj* key = *itr++, *val = *itr;
        if (m->hasKey(key))
            rt::warning("duplicate key in hashmap: " + rt::toString(key));
        m->_impl[key] = val;
    }
    return m;
}

Hashmap* Hashmap::create(hashmap_t keysvals) {
    Hashmap* m = new Hashmap();
    m->_impl = keysvals;
    return m;
}

// =========================================================================
// Constructors

Hashmap::Hashmap()
    : Fn("SxHashmap"), _impl() {
    _typeName = "SxHashmap";
    createMethods();
}

void Hashmap::clear() {
    _impl.clear();
}

void Hashmap::createMethods() {
    // ({...} key)
    FnMethod* m = addMethod(false, 1);
    m->nLocals(2);              // this, key
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_2));
    m->appendByte(static_cast<uint8_t>(vasm::GET_2 >> 8));
    m->appendByte(vasm::RETURN);
    // ({...} key not-found)
    m = addMethod(false, 2);
    m->nLocals(3);              // this, key, not-found
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_3));
    m->appendByte(static_cast<uint8_t>(vasm::GET_3 >> 8));
    m->appendByte(vasm::RETURN);
}

const hashmap_t& Hashmap::impl() const {
    return _impl;
}

// =========================================================================
// 

std::string Hashmap::toString() {
    std::stringstream ss;
    ss << "{";
    for (auto itr=_impl.begin(); itr!=_impl.end();) {
        ss << rt::toString(itr->first)
           << ' '
           << rt::toString(itr->second);
        if (++itr != _impl.end())
            ss << ", ";
    }
    ss << "}";
    return ss.str();
}

/*
size_t Hashmap::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}
*/

bool Hashmap::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Hashmap* p = pHashmap(obj)) {
        if (count() == p->count()) {
            const auto& m2 = p->impl();
            for (auto itr = _impl.begin(); itr!=_impl.end(); ++itr) {
                auto itr2 = m2.find(itr->first);
                if (itr2 != m2.end())
                    if (!rt::isEqualTo(itr->second, itr2->second))
                        return false;
            }
            return true;
        }
    }
    return false;
}

Hashmap* Hashmap::copy() {
    return Hashmap::create(_impl);
}

// =========================================================================
//

ISeq* Hashmap::seq() {
    if (isEmpty())
        return NIL;
    ISeq* ret = NIL, *tmp = NIL;
    for (auto pair : _impl)
        tmp = rt::cons(tmp, MapEntry::create(pair.first, pair.second));
    // reverse it to match the order of the printed rep of this hashmap
    for (; tmp; tmp=tmp->next())
        ret = rt::cons(ret, tmp->first());
    return ret;
}

// =========================================================================
//

int Hashmap::count() {
    return _impl.size();
}

bool Hashmap::isEmpty() {
    return _impl.empty();
}

ICollection* Hashmap::conj(Obj* obj) {
    if (MapEntry* me = dynamic_cast<MapEntry*>(obj)) {
        _impl[me->key()] = me->val();
        return this;
    }
    std::stringstream ss;
    ss << "can't conj " << rt::typeName(obj) << " onto " << _typeName;
    throw SxRuntimeError(ss.str());
}

// =========================================================================
// 

IAssociative* Hashmap::assoc(Obj* key, Obj* val) {
    _impl[key] = val;
    return this;
}

IAssociative* Hashmap::dissoc(Obj* key) {
    _impl.erase(key);
    return this;
}

bool Hashmap::hasKey(Obj* key) {
    auto itr = _impl.find(key);
    return itr != _impl.end();
}

MapEntry* Hashmap::entryAt(Obj* key) {
    auto itr = _impl.find(key);
    if (itr != _impl.end())
        return MapEntry::create(itr->first, itr->second);
    return NIL;
}

Obj* Hashmap::valAt(Obj* key, Obj* notFound) {
    auto itr = _impl.find(key);
    if (itr != _impl.end())
        return itr->second;
    return notFound;
}
