/*
  symbol.cpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#include "sxp.hpp"

Symbol* Symbol::create(const std::string& name) {
    if (name.empty())
        throw SxSyntaxError("symbol name cannot be empty");
    if (name == "/")
        return new Symbol("", "/");
    if (name[0] == '/')
        throw SxSyntaxError("symbol name cannot start with a slash: "
                            + name);
    size_t n = std::count(name.begin(), name.end(), '/');
    switch (n) {
        case 0:
            // "foo"
            return new Symbol("", name);
            break;
        case 1: {
            if (name.back() == '/')
                // "foo/"
                throw SxSyntaxError("symbol name cannot end with a single"
                                    " slash: " + name);
            // "foo/bar"
            size_t i = name.find("/");
            return new Symbol(name.substr(0, i),
                              name.substr(i + 1));
            break;
        }
        case 2: {
            if (name.find("//") != std::string::npos && name.back() == '/') {
                // foo//
                size_t i = name.find("/");
                return new Symbol(name.substr(0, i),
                                  name.substr(i + 1));
            }
            break;
        }
    }
    throw SxSyntaxError("invalid symbol name: " + name);
}

Symbol* Symbol::create(const std::string& nsName, const std::string& name) {
    if (name.empty()
        || nsName.find("/") != std::string::npos
        || (name.find("/") != std::string::npos
            && name != "/"))
        throw SxError("internal error in Symbol::create, fubar name(s)");
    return new Symbol(nsName, name);
}

Symbol::Symbol(const std::string& nsName,
               const std::string& name)
    : _nsName(nsName),
      _name(name),
      _hash(std::hash<std::string>()(_nsName.empty() ? _name
                                     : _nsName + "/" + _name)),
      _meta(nullptr) {
    _typeName = "SxSymbol";
}

// =========================================================================

std::string Symbol::toString() {
    if (_nsName.empty())
        return _name;
    return _nsName + "/" + _name;
}

size_t Symbol::getHash() {
    return _hash;
}

bool Symbol::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Symbol* sym = pSymbol(obj))
        return _nsName == sym->_nsName
            && _name == sym->_name;
    return false;
}

// =========================================================================
// IMeta

bool Symbol::hasMeta() {
    return _meta && !_meta->isEmpty();
}

Hashmap* Symbol::meta() {
    return _meta;
}

Symbol* Symbol::withMeta(Hashmap* m) {
    _meta = m;
    return this;
}

// =========================================================================
// ISortable

bool Symbol::less(Obj* obj) {
    if (this == obj)
        return false;
    Symbol* s = cpSymbol(obj);
    return toString() < s->toString();
}
