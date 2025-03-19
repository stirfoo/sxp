/*
  hashset.cpp
  S. Edward Dolan
  Friday, June 16 2023
*/

#include "sxp.hpp"

Hashset* Hashset::create() {
    return new Hashset();
}

Hashset* Hashset::create(vecobj_t objs) {
    Hashset* s = new Hashset();
    for (auto e : objs)
        s->_impl.insert(e);
    return s;
}

const hashset_t& Hashset::impl() const {
    return _impl;
}

std::string Hashset::toString() {
    std::stringstream ss;
    ss << "#{";
    for (auto itr=_impl.begin(); itr!=_impl.end();) {
        ss << rt::toString(*itr);
        if (++itr != _impl.end())
            ss << ' ';
    }
    ss << '}';
    return ss.str();
}

size_t Hashset::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool Hashset::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Hashset* p = pHashset(obj)) {
        if (count() != p->count())
            return false;
        for (auto e : _impl)
            if (!p->contains(e))
                return false;
        return true;
    }
    else if (Treeset* p = pTreeset(obj))
        return p->isEqualTo(this); // Treeset has the = implementation
    return false;
}

Hashset* Hashset::copy() {
    Hashset* s = create();
    s->_impl = _impl;
    return s;
}

ISeq* Hashset::seq() {
    if (isEmpty())
        return NIL;
    ISeq* s = List::create();
    for (auto e : _impl)
        s = s->cons(e);
    return s;
}

int Hashset::count() {
    return _impl.size();
}

bool Hashset::isEmpty() {
    return _impl.empty();
}

ICollection* Hashset::conj(Obj* x) {
    _impl.insert(x);
    return this;
}

ISet* Hashset::disjoin(Obj* x) {
    _impl.erase(x);
    return this;
}

bool Hashset::contains(Obj* x) {
    for (auto e : _impl)
        if (rt::isEqualTo(e, x))
            return true;
    return false;
}

Obj* Hashset::get(Obj* x, Obj* notFound) {
    for (auto e : _impl)
        if (rt::isEqualTo(e, x))
            return e;
    return notFound;
}

Hashmap* Hashset::meta() {
    return _meta;
}

Hashset* Hashset::withMeta(Hashmap* m) {
    _meta = m;
    return this;
}

void Hashset::createMethods() {
    // (#{...} key)
    FnMethod* m = addMethod(false, 1);
    m->nLocals(2);
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_2));
    m->appendByte(static_cast<uint8_t>(vasm::GET_2 >> 8));
    m->appendByte(vasm::RETURN);
    // (#{...} key not-found)
    m = addMethod(false, 2);
    m->nLocals(3);
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_3));
    m->appendByte(static_cast<uint8_t>(vasm::GET_3 >> 8));
    m->appendByte(vasm::RETURN);
}

Hashset::Hashset()
    : Fn("SxHashset"),
      _impl() {
    _typeName = "SxHashset";
    createMethods();
}
