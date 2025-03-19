/*
  str.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

WeakRefMap<String> String::_cache;

String* String::fetch(const std::string& val) {
    return _cache.fetch(val);
}

String* String::create(const std::string& val) {
    return new (PointerFreeGC) String(val);
}

void String::dumpCache() {
    std::cout << "=== String Cache ========================================="
              << std::endl;
    const auto& m = _cache.m();
    for (auto itr=m.begin(); itr!=m.end(); ++itr)
        if (itr->second && itr->second->_data) {
            String* str = static_cast<String*>(itr->second->_data);
            std::cout << itr->first << " => "
                      << str->toString()
                      << std::endl;
        }
}


String::String(const std::string& s)
    : _val(s),
      _hash(std::hash<std::string>()(s)) {
    _typeName = "SxString";
}

// IObj

std::string String::toString() {
    if (!rt::printReadably())
        return _val;
    std::stringstream ss;
    ss << '"';
    for (auto c : _val)
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\a': ss << "\\a"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            case '\v': ss << "\\v"; break;
            default:
                if (std::isprint(c))
                    ss << c;
                else
                    ss << "\\x" << (c & 0xff);
                break;
        }
    ss << '"';
    return ss.str();
}

size_t String::getHash() {
    return _hash;
}

bool String::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (String* s = dynamic_cast<String*>(obj))
        return s->_val == _val;
    return false;
}

// =========================================================================
// ISeqable

ISeq* String::seq() {
    ISeq* ret = NIL;
    for (auto itr = _val.crbegin(); itr!=_val.crend(); ++itr)
        ret = rt::cons(ret, Character::fetch(*itr));
    return ret;
}

// =========================================================================
// IIndexed

Obj* String::nth(int i) {
    if (i >= 0)
        for (int j=0; j<(int)_val.size(); ++j)
            if (j == i)
                return Character::fetch(_val[i]);
    std::stringstream ss;
    ss << _typeName << " index (" << i << ") out of bounds";
    throw SxOutOfBoundsError(ss.str());
}

Obj* String::nth(int i, Obj* notFound) {
    for (int j=0; j<(int)_val.size(); ++j)
        if (j == i)
            return Character::fetch(_val[i]);
    return notFound;
}

// =========================================================================
// ICollection

int String::count() {
    return _val.size();
}

bool String::isEmpty() {
    return _val.empty();
}

ICollection* String::conj(Obj* obj) {
    if (Character* c = dynamic_cast<Character*>(obj)) {
        String* s = new String(_val);
        s += c->val();
        return s;
    }
    std::stringstream ss;
    ss << "can't conj " << obj->typeName() << " onto a String";
    throw SxRuntimeError(ss.str());
}

// =========================================================================
// ISortable

bool String::less(Obj* obj) {
    if (this == obj)
        return false;
    String* s = cpString(obj);
    return _val < s->val();
}
