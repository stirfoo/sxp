/*
  char.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

std::vector<Character*, gc_allocator<Character*>> Character::_cache;

void Character::init() {
    _cache.resize(256);
}

void Character::shutdown() {
    _cache.clear();
}

Character* Character::fetch(int c) {
    if (_cache[c])
        return _cache[c];
    return _cache[c] = new (PointerFreeGC) Character(c);
}

int Character::val() {
    return _val;
}

Character::Character(int c)
    : _val(c),
      _hash(std::hash<char>()(c)) {
    _typeName = "SxCharacter";
}

std::string Character::toString() {
    std::stringstream s;
    if (!rt::printReadably())
        s << (char)(_val & 0xff);
    else {
        s << "\\";
        switch (_val) {
            case ' ': s << "space"; break;
            case '\t': s << "tab"; break;
            case '\r': s << "return"; break;
            case '\n': s << "newline"; break;
            case '\b': s << "backspace"; break;
            case '\f': s << "formfeed"; break;
            case '\v': s << "vtab"; break;
            case '\a': s << "bell"; break;
            case '\\': s << "\\"; break;
            default:
                if (std::isprint(_val))
                    s << (char)(_val & 0xff);
                else
                    s << "x" << std::hex << (_val & 0xff);
        }
    }
    return s.str();
}

size_t Character::getHash() {
    return _hash;
}

bool Character::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Character* c = dynamic_cast<Character*>(obj))
        return _val == c->_val;
    return false;
}

// =========================================================================
// ISortable

bool Character::less(Obj* obj) {
    if (this == obj)
        return false;
    Character* p = cpCharacter(obj);
    return _val < p->val();
}
