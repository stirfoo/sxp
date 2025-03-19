/*
  bool.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

Bool* Bool::TRUE = nullptr, *Bool::FALSE = nullptr;

Bool* Bool::create(bool val) {
    if (val == true) {
        if (TRUE)
            return TRUE;
        else
            return TRUE = new Bool(val);
    }
    else if (FALSE)
        return FALSE;
    else
        return FALSE = new Bool(val);
}

Bool::Bool(bool val)
    : _val(val), _hash(val ? 1387 : 1385) {
    _typeName = "SxBool";
}

std::string Bool::toString() {
    return _val ? "true" : "false";
}

size_t Bool::getHash() {
    return _hash;
}

// =========================================================================
// ISortable

bool Bool::less(Obj* obj) {
    if (this == obj)
        return false;
    Bool* p = cpBool(obj);
    return !_val && p->_val;   // false < true
}
