/*
  vector.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

Vector* Vector::create() {
    return new Vector();
}

Vector* Vector::create(const vecobj_t& v) {
    return new Vector(v);
}

// ctors

Vector::Vector() : Fn("SxVector"), _impl() {
    _typeName = "SxVector";
    createMethods();
}

Vector::Vector(const vecobj_t& v) : Fn("SxVector"), _impl(v) {
    _typeName = "SxVector";
    createMethods();
}

// 

void Vector::createMethods() {
    // ([] i)
    FnMethod* m = addMethod(false, 1);
    m->nLocals(2);            // this, index
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::NTH_2));
    m->appendByte(static_cast<uint8_t>(vasm::NTH_2 >> 8));
    m->appendByte(vasm::RETURN);
    // ([] i not-found)
    m = addMethod(false, 2);
    m->nLocals(3);            // this, index, notFound
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::NTH_3));
    m->appendByte(static_cast<uint8_t>(vasm::NTH_3 >> 8));
    m->appendByte(vasm::RETURN);
}

const vecobj_t& Vector::impl() const {
    return _impl;
}

// ========================================================================
// 

std::string Vector::toString() {
    std::stringstream ss;
    ss << '[';
    size_t n = _impl.size();
    for (size_t i=0; i<n; ++i) {
        ss << rt::toString(_impl[i]);
        if (i < n-1)
            ss << ' ';
    }
    ss << ']';
    return ss.str();
}

size_t Vector::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool Vector::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    else if (Vector* v = dynamic_cast<Vector*>(obj)) {
        if (count() == v->count())
            return std::equal(_impl.begin(), _impl.end(), v->_impl.begin(),
                              objEqual);
    }
    else if (List* p = pList(obj)) {
        int i = 0;
        ISeq* s = p->seq();
        for (; i<count()&&s; ++i, s=s->next())
            if (!rt::isEqualTo(nth(i), s->first()))
                return false;
        return i == count() && !s; // all compared
    }
    else if (MapEntry* p = pMapEntry(obj)) {
        return count() == 2
            && rt::isEqualTo(nth(0), p->key())
            && rt::isEqualTo(nth(1), p->val());
    }
    return false;
}

// =========================================================================
// ISeqable

ISeq* Vector::seq() {
    if (_impl.empty())
        return NIL;
    ISeq* ret = NIL;
    for (auto itr=_impl.crbegin(); itr!=_impl.crend(); ++itr)
        ret = rt::cons(ret, *itr);
    return ret;
}

// =========================================================================
// IIndexed

Obj* Vector::nth(int i) {
    if (i >= 0)
        for (int j=0; j<(int)_impl.size(); ++j)
            if (j == i)
                return _impl[i];
    std::stringstream ss;
    ss << _typeName << " index (" << i << ") out of bounds";
    throw SxOutOfBoundsError(ss.str());
}

Obj* Vector::nth(int i, Obj* notFound) {
    for (int j=0; j<(int)_impl.size(); ++j)
        if (j == i)
            return _impl[i];
    return notFound;
}

// =========================================================================
// ICollection

int Vector::count() {
    return _impl.size();
}

bool Vector::isEmpty() {
    return _impl.empty();
}

ICollection* Vector::conj(Obj* obj) {
    _impl.push_back(obj);
    return this;
}
