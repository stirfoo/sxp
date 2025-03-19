/*
  treeset.cpp
  S. Edward Dolan
  Friday, June 16 2023
*/

#include "sxp.hpp"

Treeset* Treeset::create() {
    return new Treeset();
}

Treeset* Treeset::create(vecobj_t objs) {
    Treeset* s = new Treeset();
    for (auto e : objs)
        s->_impl.insert(e);
    return s;
}

const treeset_t& Treeset::impl() const {
    return _impl;
}

// TODO: again, do I use the same readably rep as a hashset?
std::string Treeset::toString() {
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

size_t Treeset::getHash() {
    std::stringstream ss;
    ss << typeName() << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool Treeset::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Treeset* p = pTreeset(obj)) {
        if (count() != p->count())
            return false;
        for (auto e : _impl)
            if (!p->contains(e))
                return false;
        return true;
    }
    else if (Hashset* p = pHashset(obj)) {
        if (count() != p->count())
            return false;
        for (auto e : _impl)
            if (!p->contains(e))
                return false;
        return true;
    }
    return false;
}

Treeset* Treeset::copy() {
    Treeset* s = create();
    s->_impl = _impl;
    return s;
}

ISeq* Treeset::seq() {
    if (isEmpty())
        return NIL;
    Vector* v = Vector::create(); // A list would cons in reverse order
    for (auto e : _impl)
        v->conj(e);
    return v->seq();
}

int Treeset::count() {
    return _impl.size();
}

bool Treeset::isEmpty() {
    return _impl.empty();
}

ICollection* Treeset::conj(Obj* x) {
    if (_impl.empty())
        _impl.insert(cpISortable(x));
    else
        _impl.insert(x);
    return this;
}

ISet* Treeset::disjoin(Obj* x) {
    _impl.erase(x);
    return this;
}

bool Treeset::contains(Obj* x) {
    for (auto e : _impl)
        if (rt::isEqualTo(e, x))
            return true;
    return false;
}

Obj* Treeset::get(Obj* x, Obj* notFound) {
    for (auto e : _impl)
        if (rt::isEqualTo(e, x))
            return e;
    return notFound;
}

Hashmap* Treeset::meta() {
    return _meta;
}

Obj* Treeset::withMeta(Hashmap* m) {
    _meta = m;
    return this;
}

void Treeset::createMethods() {
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

Treeset::Treeset()
    : Fn("SxTreeset"),
      _impl() {
    _typeName = "SxTreeset";
    createMethods();
}
