/*
  keyword.cpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#include "sxp.hpp"

WeakRefMap<Keyword> Keyword::_cache;

Keyword* Keyword::fetch(const std::string& name) {
    /*
      false means don't use `new (PointerFreGC)' because the methods of the
      parent Fn will get collected prematurely.
    */
    return _cache.fetch(name, false);
}

void Keyword::dumpCache() {
    std::cout << "=== Keyword Cache ========================================="
              << std::endl;
    const auto& m = _cache.m();
    for (auto itr=m.begin(); itr!=m.end(); ++itr)
        if (itr->second && itr->second->_data) {
            Keyword* kwd = static_cast<Keyword*>(itr->second->_data);
            std::cout << itr->first << " => "
                      << kwd->toString()
                      << std::endl;
        }
}

Keyword::Keyword(const std::string& name)
    : Fn("SxKeyword"),
      _name(name),
      _hash(std::hash<std::string>()(":" + name)) {
    _typeName = "SxKeyword";
    createMethods();
}

void Keyword::createMethods() {
    // (:foo map)
    FnMethod* m = addMethod(false, 1);
    m->nLocals(2);              // this, map
    m->appendByte(vasm::SWAP);
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_2));
    m->appendByte(static_cast<uint8_t>(vasm::GET_2 >> 8));
    m->appendByte(vasm::RETURN);
    // (:foo map not-found)
    m = addMethod(false, 2);
    m->nLocals(3);              // this, map, not-found
    m->appendByte(vasm::SWAP2);
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(vasm::GET_3));
    m->appendByte(static_cast<uint8_t>(vasm::GET_3 >> 8));
    m->appendByte(vasm::RETURN);
}

// =========================================================================

std::string Keyword::toString() {
    return ":" + _name;
}

size_t Keyword::getHash() {
    return _hash;
}


// =========================================================================
// ISortable

bool Keyword::less(Obj* obj) {
    if (this == obj)
        return false;
    Keyword* p = cpKeyword(obj); // may throw
    return toString() < p->toString();
}
