/*
  obj.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

Obj* Obj::create() {
    return new (PointerFreeGC) Obj();
}

std::string Obj::typeName() const {
    return _typeName;
}

std::string Obj::toString() {
    std::stringstream ss;
    ss << "#<" << _typeName << " " << this << ">"; // #<_typename address>
    return ss.str();
}

size_t Obj::getHash() {
    return std::hash<Obj*>()(this);
}

bool Obj::isEqualTo(Obj* obj) {
    return this == obj;         // default compare by identity
}

Obj* Obj::copy() {
    throw SxNotImplementedError(typeName() + " is not copy-able");
}

// Has to be implemented here because xface.hpp is included after obj.hpp
bool ObjLessFntr::operator()(Obj* o1, Obj* o2) const {
    if (!o1) {
        if (!o2)
            return false;               // nil == nil
        return cpISortable(o2) && true; // nil < o2
    }
    else if (!o2)
        return false;           // o2 > nil
    return cpISortable(o1)->less(cpISortable(o2)); // o1 < o2 ?
}

