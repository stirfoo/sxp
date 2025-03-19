/*
  treemap.cpp
  S. Edward Dolan
  Saturday, June 17 2023
*/

#include "sxp.hpp"

// =========================================================================
// static creation

Treemap* Treemap::create() {
    return new Treemap();
}

Treemap* Treemap::create(const vecobj_t& v) {
    assert((v.size() % 2) == 0 && "TREEMAP K/V VECTOR MISSING FINAL VALUE");
    Treemap* m = new Treemap();
    if (v.size() == 0)
        return m;
    for (auto itr=v.begin(); itr!=v.end(); ++itr) {
        Obj* key = *itr++, *val = *itr;
        if (m->hasKey(key))
            rt::warning("duplicate key in treemap: " + rt::toString(key));
        m->_impl[key] = val;
    }
    return m;
}

Treemap* Treemap::create(treemap_t keysvals) {
    Treemap* m = new Treemap();
    m->_impl = keysvals;
    return m;
}

// =========================================================================
// Constructors

Treemap::Treemap()
    : Fn("SxTreemap"), _impl() {
    _typeName = "SxTreemap";
    createMethods();
}

void Treemap::clear() {
    _impl.clear();
}

void Treemap::createMethods() {
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

const treemap_t& Treemap::impl() const {
    return _impl;
}

ISeq* Treemap::rseq() {
    if (isEmpty())
        return NIL;
    ISeq* s = NIL;
    for (auto pair : _impl)
        s = rt::cons(s, MapEntry::create(pair.first, pair.second));
    return s;
}

// =========================================================================
// 

//  TODO: ok to print with the same reader syntax as a hashmap?
std::string Treemap::toString() {
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

size_t Treemap::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool Treemap::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Treemap* p = pTreemap(obj)) {
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

Treemap* Treemap::copy() {
    return create(_impl);
}

// =========================================================================
//

ISeq* Treemap::seq() {
    if (isEmpty())
        return NIL;
    ISeq* ret = NIL, *tmp = NIL;
    for (auto pair : _impl)
        tmp = rt::cons(tmp, MapEntry::create(pair.first, pair.second));
    // reverse it to match the order of the printed rep of this treemap
    for (; tmp; tmp=tmp->next())
        ret = rt::cons(ret, tmp->first());
    return ret;
}

// =========================================================================
//

int Treemap::count() {
    return _impl.size();
}

bool Treemap::isEmpty() {
    return _impl.empty();
}

ICollection* Treemap::conj(Obj* obj) {
    if (MapEntry* me = dynamic_cast<MapEntry*>(obj)) {
        _impl[cpISortable(me->key())] = me->val();
        return this;
    }
    std::stringstream ss;
    ss << "can't conj " << rt::typeName(obj) << " onto " << typeName();
    throw SxRuntimeError(ss.str());
}

// =========================================================================
// 

IAssociative* Treemap::assoc(Obj* key, Obj* val) {
    _impl[cpISortable(key)] = val;
    return this;
}

IAssociative* Treemap::dissoc(Obj* key) {
    _impl.erase(cpISortable(key));
    return this;
}

bool Treemap::hasKey(Obj* key) {
    auto itr = _impl.find(cpISortable(key));
    return itr != _impl.end();
}

MapEntry* Treemap::entryAt(Obj* key) {
    auto itr = _impl.find(cpISortable(key));
    if (itr != _impl.end())
        return MapEntry::create(itr->first, itr->second);
    return NIL;
}

Obj* Treemap::valAt(Obj* key, Obj* notFound) {
    auto itr = _impl.find(cpISortable(key));
    if (itr != _impl.end())
        return itr->second;
    return notFound;
}
