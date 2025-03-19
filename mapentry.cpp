/*
  mapentry.cpp
  S. Edward Dolan
  Saturday, April  8 2023
*/

#include "sxp.hpp"

MapEntry* MapEntry::create(Obj* key, Obj* val) {
    return new MapEntry(key, val);
}

MapEntry::MapEntry(Obj* key, Obj* val)
    : _key(key),
      _val(val) {
    _typeName = "SxMapEntry";
}

Obj* MapEntry::key() {
    return _key;
}

Obj* MapEntry::val() {
    return _val;
}

std::string MapEntry::toString() {
    std::stringstream ss;
    ss << '[';
    ss << rt::toString(_key) << ' ' << rt::toString(_val);
    ss << ']';
    return ss.str();
}

size_t MapEntry::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool MapEntry::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    else if (MapEntry* me = pMapEntry(obj))
        return rt::isEqualTo(_key, me->key())
            && rt::isEqualTo(_val, me->val());
    else if (Vector* v = pVector(obj))
        return v->isEqualTo(this);
    else if (List* l = pList(obj))
        return l->isEqualTo(this);
    return false;
}

MapEntry* MapEntry::copy() {
    return create(_key, _val);
}

// =========================================================================

ISeq* MapEntry::seq() {
    return List::create(_key, _val);
}

// =========================================================================

Obj* MapEntry::nth(int i) {
    if (i == 0)
        return _key;
    else if (i == 1)
        return _val;
    std::stringstream ss;
    ss << _typeName << " index (" << i << ") out of bounds";
    throw SxOutOfBoundsError(ss.str());
}

Obj* MapEntry::nth(int i, Obj* notFound) {
    if (i == 0)
        return _key;
    else if (i == 1)
        return _val;
    else
        return notFound;
}

// =========================================================================

int MapEntry::count() {
    return 2;
}

bool MapEntry::isEmpty() {
    return false;
}

ICollection* MapEntry::conj(Obj* x) {
    return Vector::create({_key, _val, x});
}
