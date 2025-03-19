/*
  proc_ns.cpp
  S. Edward Dolan
  Wednesday, June 21 2023

  This file is included in vm.cpp
*/

// ... proc ns]
// ... proc ns hashmap]
case vasm::NS_MAP_1: {
    ppush(cpNamespace(ppeek())->bindings());
    break;
}
// ... proc ns]
// ... proc ns sym]
case vasm::NS_NAME_1: {
    ppush(cpNamespace(ppeek())->name());
    break;
}
// ... proc sym]
// ... proc sym ns]
case vasm::IN_NS_1: {
    Namespace* ns = Namespace::fetch(cpSymbol(ppeek(0)));
    ppush(rt::currentNS(ns));
    break;
}
// ... proc sym]
// ... proc sym nil]
case vasm::REFER_1: {
    Symbol* sym = cpSymbol(ppeek());
    if (Namespace* ns = Namespace::find(sym)) {
        rt::currentNS()->refer(ns);
        ppush(NIL);
    }
    else
        throw SxRuntimeError("can't refer to a namespace ("
                             + sym->toString() + ") that does not exist");
    break;
}
// ... proc sym]
// ... proc sym ns-or-nil]
case vasm::FIND_NS_1: {
    ppush(Namespace::find(cpSymbol(ppeek())));
    break;
}
// ... proc sym]
// ... proc sym map]
case vasm::NS_PUBLICS_1: {
    Symbol* sym = cpSymbol(ppeek());
    if (Namespace* ns = Namespace::find(sym)) {
        Hashmap* m = Hashmap::create();
        for (auto e : ns->bindings()->impl()) {
            if (Var* v = pVar(e.second))
                if (v->ns() == ns && v->isPublic())
                    m->assoc(e.first, e.second);
        }
        ppush(m);
    }
    else
        throw SxRuntimeError("namespace (" + sym->toString()
                             + ") does not exist");
    break;
}

