/*
  lazyseq.cpp
  S. Edward Dolan
  Saturday, June 17 2023
*/

#include "sxp.hpp"

LazySeq* LazySeq::create(Obj* fn) {
    return new LazySeq(fn);
}

LazySeq::LazySeq(Obj* fn)
    : _fn(fn),
      _sv(nullptr),
      _s(nullptr) {
    _typeName = "SxLazySeq";
}

std::string LazySeq::toString() {
    std::stringstream ss;
    ss << '(';
    for (ISeq* s=seq(); s; s=s->next()) {
        ss << rt::toString(s->first());
        if (s->next())
            ss << ' ';
    }
    ss << ')';
    return ss.str();
}

size_t LazySeq::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool LazySeq::isEqualTo(Obj* x) {
    if (this == x)
        return true;
    return false;
}

Obj* LazySeq::sval() {
    if (_fn) {
        VM vm;
        _sv = vm.run(_fn);
        _fn = NIL;
    }
    if (_sv)
        return _sv;
    return _s;
}

ISeq* LazySeq::seq() {
    sval();
    if (_sv) {
        Obj* ls = _sv;
        _sv = NIL;
        while (pLazySeq(ls))
            ls = pLazySeq(ls)->sval();
        _s = ls ? NIL : pLazySeq(ls)->seq();
    }
    return _s;
}

Obj* LazySeq::first() {
    seq();
    if (!_s)
        return NIL;
    return _s->first();
}

ISeq* LazySeq::rest() {
    seq();
    if (!_s)
        return List::create();
    return _s->rest();
}

ISeq* LazySeq::next() {
    seq();
    if (!_s)
        return NIL;
    return _s->next();
}

ISeq* LazySeq::cons(Obj* x) {
    return rt::cons(seq(), x);
}

int LazySeq::count() {
    int c = 0;
    for (ISeq* s=seq(); s; s=s->next())
        ++c;
    return c;
}

bool LazySeq::isEmpty() {
    return seq() == NIL;
}

ICollection* LazySeq::conj(Obj* x) {
    return pList(rt::cons(seq(), x));
}

