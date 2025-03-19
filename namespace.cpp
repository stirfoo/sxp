/*
  namespace.cpp
  S. Edward Dolan
  Thursday, May 11 2023
*/

#include "sxp.hpp"

Hashmap* Namespace::_namespaces = nullptr;

void Namespace::init() {
    _namespaces = Hashmap::create();
}

void Namespace::shutdown() {
    _namespaces->clear();
    _namespaces = nullptr;
}

Namespace* Namespace::find(Symbol* sym) {
    return pNamespace(_namespaces->valAt(sym));
}

Namespace* Namespace::fetch(Symbol* sym) {
    if (!sym->_nsName.empty())
        throw SxRuntimeError("invalid namespace name, cannot be"
                             " ns-qualified: " + sym->toString());
    Namespace* ns = pNamespace(_namespaces->valAt(sym, NIL));
    if (!ns) {
        ns = new Namespace(sym);
        _namespaces->assoc(sym, ns);
    }
    // std::cout << "ALL:" << rt::toString(_namespaces) << std::endl;
    return ns;
}

std::string Namespace::toString() {
    std::stringstream ss;
    ss << "#<" << typeName() << ' ' << name()->name()
       << ' ' << this << '>';
    return ss.str();
}

Namespace::Namespace(Symbol* name)
    : _name(name),
      _bindings(Hashmap::create()),
      _aliases(Hashmap::create()) {
    for (auto e : rt::DEFAULT_IMPORTS->impl())
        _bindings->assoc(e.first, e.second);
    _typeName = "SxNamespace";
}

Var* Namespace::intern(Symbol* sym) {
    if (!sym->_nsName.empty())
        throw SxRuntimeError("can't intern ns-qualified symbol: "
                             + sym->toString());
    Var* newVar = Var::create(this, sym, rt::UNBOUND);
    Obj* curVar = _bindings->valAt(sym);
    if (curVar) {
        if (rt::isEqualTo(curVar, this)) {
            newVar = nullptr;   // gc
            return pVar(curVar);
        }
        else
            warnOrFailOnReplace(sym, pVar(curVar), newVar);
    }
    _bindings->assoc(sym, newVar);
    return newVar;
}

/*
  Called if a newly interned symbol (SYM) is already bound to a var with a
  different namespace than the one it's being interned into.
*/
void Namespace::warnOrFailOnReplace(Symbol* sym, Obj* curVal, Obj* newVal) {
    if (Var* p = pVar(curVal)) {
        Namespace* ns = p->ns();
        if (ns == this)
            return;
        if (ns != rt::sxpNS())
            throw SxRuntimeError(sym->toString() + " already refers to: " +
                                 rt::toString(curVal) + " in namespace: " +
                                 _name->toString());
    }
    std::stringstream ss;
    ss << sym->toString() << " already refers to: "
       << rt::toString(curVal) << " in namespace: "
       << _name->toString() << ", being replaced by: "
       << rt::toString(newVal) << std::endl;
    rt::warning(ss.str());
#if 0        
    if (curVar->ns() == this)   // compare by identity
        return;
    if (curVar->ns() != rt::currentNS())
        throw SxRuntimeError(sym->toString() + " already refers to: " +
                             curVar->toString() + " in namespace: " +
                             _name->toString());
    std::stringstream ss;
    ss << sym->toString() << " already refers to: "
       << curVar->toString() << " in namespace: "
       << _name->toString() << ", being replaced by: "
       << newVal->toString() << std::endl;
    rt::warning(ss.str());
#endif    
}

Namespace* Namespace::lookupAlias(Symbol* sym) {
    return pNamespace(_aliases->valAt(sym));
}

Var* Namespace::findInternedVar(Symbol* sym) {
    Var* var = pVar(_bindings->valAt(sym));
    if (var && var->ns() == this)
        return var;
    return nullptr;
}

Obj* Namespace::get(Symbol* sym) {
    return _bindings->valAt(sym);
}

void Namespace::refer(Namespace* src) {
    if (this == src)
        return;
    for (auto e : src->bindings()->impl())
        if (Var* p = pVar(e.second))
            if (p->isPublic())
                reference(pSymbol(e.first), e.second);
}

void Namespace::reference(Symbol* sym, Obj* val) {
    if (sym->hasNS())
        throw SxRuntimeError("can't intern ns-qualified symbol: "
                             + sym->toString());
    if (Obj* oldVal = get(sym)) {
        if (rt::isEqualTo(oldVal, val))
            return;
        else
            warnOrFailOnReplace(sym, oldVal, val);
    }
    _bindings->assoc(sym, val);
}
