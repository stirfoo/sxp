/*
  var.cpp
  S. Edward Dolan
  Thursday, May 11 2023
*/

#include "sxp.hpp"

// =========================================================================
// Var

vecobj_t Var::_dynVars;

Var::Var(Namespace* ns, Symbol* sym, Obj* root)
    : _ns(ns),
      _sym(sym),
      _rootVal(root),
      _dynVals(),
      _meta(Hashmap::create()) {
    _typeName = "SxVar";
}

Var* Var::intern(Namespace* ns, Symbol* sym, Obj* root) {
    Var* v = ns->intern(sym);
    v->_rootVal = root;
    return v;
}

Var* Var::create() {
    return create(rt::UNBOUND);
}

Var* Var::create(Obj* val) {
    return new Var(nullptr, nullptr, val);
}

Var* Var::create(Namespace* ns, Symbol* sym, Obj* root) {
    return new Var(ns, sym, root);
}

void Var::pushBindings(Hashmap* m) {
    for (auto itr=m->impl().begin(); itr!=m->impl().end(); ++itr)
        pVar(itr->first)->pushDyn(itr->second);
    _dynVars.push_back(rt::keys(m));
}

void Var::popBindings() {
    ISeq* s = pISeq(_dynVars.back());
    for (; s; s=s->next())
        pVar(s->first())->popDyn();
    _dynVars.pop_back();
}

std::string Var::toString() {
    std::stringstream ss;
    ss << "#'" << _ns->name()->toString() << '/'
       << _sym->toString();
    return ss.str();
}

Obj* Var::get() {
    if (isDynBound())
        return _dynVals.back();
    return _rootVal;
}

Obj* Var::set(Obj* x) {
    if (isDynBound())
        return _dynVals.back() = x;
    else
        return setRoot(x, true);
    // // if (isMacro())
    // //     _meta->dissoc(rt::KW_MACRO);
    // return _rootVal = x;
}

Obj* Var::setRoot(Obj* x, bool resetMacro) {
    if (isDynBound())
        rt::warning("rebinding root value of currently dynamically bound"
                    " var: " + toString());
    if (resetMacro)
        _meta->dissoc(rt::KW_MACRO);
    return _rootVal = x;
}

void Var::pushDyn(Obj* x) {
    if (!isDynamic())
        throw SxError(toString() + " is not a dynamic var");
    _dynVals.push_back(x);
}

void Var::popDyn() {
    if (!isDynamic())
        throw SxError(toString() + " is not a dynamically bound var");
    assert(!_dynVals.empty());
    _dynVals.pop_back();
}

void Var::setDynamic() {
    _meta->assoc(rt::KW_DYNAMIC, rt::T);
}

bool Var::isDynamic() {
    return rt::toBool(_meta->valAt(rt::KW_DYNAMIC));
}

bool Var::isBound() {
    return (_rootVal != rt::UNBOUND) || isDynBound();
}

bool Var::isMacro() {
    return rt::toBool(_meta->valAt(rt::KW_MACRO));
}

bool Var::isPublic() {
    return !rt::toBool(_meta->valAt(rt::KW_PRIVATE));
}

Var* Var::withMeta(Hashmap* m) {
    if (rt::isEqualTo(m->valAt(rt::KW_TAG), rt::KW_DYNAMIC))
        m->dissoc(rt::KW_TAG)->assoc(rt::KW_DYNAMIC, rt::T);
    _meta = m;
    return this;
}
