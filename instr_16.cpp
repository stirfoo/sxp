/*
  instr_16.cpp
  S. Edward Dolan
  Wednesday, May 24 2023
*/

// ... proc x]
// ... proc x list-or-nil]
case vasm::SEQ_1: {
    ppush(rt::seq(ppeek()));
    break;
}
// ... proc x]
// ... proc x y]
case vasm::FIRST_1: {
    ppush(rt::first(ppeek()));
    break;
}
// ... proc x]
// ... proc x y]
case vasm::REST_1: {
    ppush(rt::rest(ppeek()));
    break;
}
// ... proc x]
// ... proc x y]
case vasm::NEXT_1: {
    ppush(rt::next(ppeek()));
    break;
}
// ... proc coll x list-or-nil]
// ... proc coll x list-or-nil coll']
case vasm::CONJ_2N: {
    ICollection* c = rt::conj(ppeek(2), ppeek(1));
    for (ISeq* s=pISeq(ppeek()); s; s=s->next())
        c = rt::conj(c, s->first());
    ppush(c);
    break;
}
// ... proc (a1 a2 ... aN)
// ... proc (a1 a2 ... aN) list
case vasm::CONCAT_0N: {
    Vector* v = Vector::create();
    if (ppeek())
        for (ISeq* r=pISeq(ppeek()); r; r=r->next())
            for (ISeq* s=rt::seq(r->first()); s; s=s->next())
                v->conj(s->first());
    ppush(v->impl().empty() ? List::create() : v->seq());
    break;
}
// ... proc (a1 a2 ... aN)]
// ... proc (a1 a2 ... aN) (a1 a2 ... aN)]
case vasm::LIST_0N: {
    if (!ppeek())
        ppush(List::create());
    else
        ppush(ppeek());
    break;
}
// ... proc a1 a2 ... aN]
// ... proc a1 a2 ... aN [a1 a2 ... aN]]
case vasm::VECTOR_0N: {
    Vector* v = Vector::create();
    if (ppeek())
        for (ISeq* s=pISeq(ppeek()); s!=NIL; s=s->next())
            v->conj(s->first());
    ppush(v);
    break;
}
// ... proc k1 v1 k2 v2 ... kN vN]
// ... proc k1 v1 k2 v2 ... kN vN {k1 v1 kN vN k2 v2}]
case vasm::HASHMAP_0N: {
    Hashmap* m = Hashmap::create();
    if (ppeek()) {
        for (ISeq* s=pISeq(ppeek()); s; s=s->next()) {
            Obj* key = s->first();
            if (!(s=s->next()))
                throw SxRuntimeError("hashmap missing final value");
            m->assoc(key, s->first());
        }
    }
    ppush(m);
    break;
}
// ... proc list-or-nil]
// ... proc list-or-nil #{e1 e2 ... eN}]
case vasm::HASHSET_0N: {
    Hashset* hs = Hashset::create();
    if (ppeek())
        for (ISeq* s=pISeq(ppeek()); s; s=s->next())
            hs->conj(s->first());
    ppush(hs);
    break;
}
// ... proc list-or-nil]
// ... proc list-or-nil {k1 v1 kN vN k2 v2}]
case vasm::TREEMAP_0N: {
    Treemap* m = Treemap::create();
    for (ISeq* s=pISeq(ppeek()); s; s=s->next()) {
        Obj* key = s->first();
        if (!(s=s->next()))
            throw SxRuntimeError("treemap missing final value");
        m->assoc(key, s->first());
    }
    ppush(m);
    break;
}
// ... proc list-or-nil]
// ... proc list-or-nil #{...}]
case vasm::TREESET_0N: {
    Treeset* m = Treeset::create();
    for (ISeq* s=pISeq(ppeek()); s; s=s->next())
        m->conj(s->first());
    ppush(m);
    break;
}
// ... proc x]
// ... proc x string]
case vasm::TYPENAME_1: {
    ppush(String::fetch(rt::typeName(ppeek())));
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::EQ_1: {
    ppush(rt::T);
    break;
}
// ... proc x y]
// ... proc x y bool]
case vasm::EQ_2: {
    ppush(rt::isEqualTo(ppeek(1), ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x y list-or-nil]
// ... proc x y list-or-nil bool]
case vasm::EQ_2N: {
    Obj* x = ppeek(2), *y = ppeek(1);
    if (!rt::isEqualTo(x, y)) {
        ppush(rt::F);
        goto fail;
    }
    x = y;
    for (ISeq* s=rt::seq(ppeek()); s; s=s->next())
        if (!rt::isEqualTo(x, s->first())) {
            ppush(rt::F);
            goto fail;
        }
    ppush(rt::T);
 fail:
    break;
}
// ... proc]
// ... proc nil]
case vasm::VM_TRACE_0: {
    rt::vmTrace = !rt::vmTrace;
    ppush(NIL);
    break;
}
// ... proc]
// ... proc nil]
case vasm::VM_STACK_0: {
    printStack(cpIOutStream(rt::currentOUT())->ostream());
    ppush(NIL);
    break;
}
// ... proc form]
// ... proc form fn]
case vasm::COMPILE_1: {
    ppush(compiler::compile(ppeek()));
    break;
}
// ... proc x y]
// ... proc x y bool]
case vasm::IDENTICAL_P_2: {
    ppush(ppeek(1) == ppeek() ? rt::T : rt::F);
    break;
}
// ... proc]
// ... proc empty-string]
case vasm::STR_0: {
    ppush(String::fetch(""));
    break;
}
// ... proc x]
// ... proc x string-or-nil]
case vasm::STR_1: {
    Obj* x = ppeek();
    if (x == NIL)
        ppush(String::fetch(""));
    else {
        DynScope ds(rt::VAR_PRINT_READABLY, rt::F);
        ppush(String::create(rt::toString(x)));
    }
    break;
}
// ... proc x list-or-nil]
// ... proc x list-or-nil string]
case vasm::STR_1N: {
    std::stringstream ss;
    DynScope ds(rt::VAR_PRINT_READABLY, rt::F);
    for (ISeq* s = rt::cons(ppeek(), ppeek(1)); s; s=s->next())
        if (s->first())
            ss << rt::toString(s->first());
    ppush(String::create(ss.str())); // TODO: fetch or create?
    break;
}
// ... proc coll i]
// ... proc coll i ith-obj]
case vasm::NTH_2: {
    ppush(rt::nth(ppeek(1), cpINumber(ppeek())->toInt()));
    break;
}
// ... proc coll i not-found]
// ... proc coll i not-found ith-obj-or-not-found]
case vasm::NTH_3: {
    ppush(rt::nth(ppeek(2), cpINumber(ppeek(1))->toInt(), ppeek()));
    break;
}
// ... proc coll key]
// ... proc coll key val-or-nil]
case vasm::GET_2: {
    ppush(rt::get(ppeek(1), ppeek()));
    break;
}
// ... proc coll key not-found]
// ... proc coll key not-found val-or-not-found]
case vasm::GET_3: {
    ppush(rt::get(ppeek(2), ppeek(1), ppeek()));
    break;
}
// ... proc map key val list-or-nil]
// ... proc map key val list-or-nil map']
case vasm::ASSOC_3N: {
    ISeq* s = pISeq(ppeek());
    if (rt::count(s) % 2)
        throw SxRuntimeError("ASSOC missing final value argument");
    IAssociative* m = rt::assoc(ppeek(3), ppeek(2), ppeek(1));
    for (; s; s=s->next()) {
        Obj* key = s->first();
        s = s->next();
        m = m->assoc(key, s->first());
    }
    ppush(m);
    break;
}
// ... proc map list-or-nil]
// ... proc map list-or-nil map'-or-nil]
case vasm::DISSOC_1N: {
    if (!ppeek(1))
        ppush(NIL);
    else {
        IAssociative* m = cpIAssociative(ppeek(1));
        for (ISeq* s=pISeq(ppeek()); s; s=s->next())
            m->dissoc(s->first());
        ppush(m);
    }
    break;
}
// ... proc fn]
// ... proc fn lazyseq]
case vasm::MAKE_LAZY_SEQ_1: {
    // std::cout << "MAKE_LAZY_SEQ_1:" << rt::toString(ppeek())
    //           << std::endl;
    ppush(LazySeq::create(ppeek()));
    break;
}
// ... proc name]
// ... proc name keyword]
case vasm::KEYWORD_1: {
    if (String* p = pString(ppeek()))
        ppush(Keyword::fetch(p->val()));
    else if (Symbol* p = pSymbol(ppeek()))
        // (keyword 'foo/bar)    => :foo/bar
        ppush(Keyword::fetch(p->toString()));
    else
        throw SxIllegalArgumentError("KEYWORD wants a string or symbol name");
    break;
}
// ... proc name]
// ... proc name symbol]
case vasm::SYMBOL_1: {
    ppush(Symbol::create(cpString(ppeek())->val()));
    break;
}
// ... proc ns-name name]
// ... proc ns-name name symbol]
case vasm::SYMBOL_2: {
    std::string name = cpString(ppeek())->val();
    ppush(Symbol::create(cpString(ppeek(1))->val(), name));
    break;
}
// ... proc]
// ... proc integer]
case vasm::NEXT_ID_0: {
    ppush(Integer::fetch(rt::nextID()));
    break;
}
// ... proc obj]
// ... proc obj map]
case vasm::META_1: {
    ppush(cpIMeta(ppeek())->meta());
    break;
}
// ... proc obj map-or-nil]
// ... proc obj map-or-nil obj']
case vasm::WITH_META_2: {
    Obj* m = ppeek();
    if (!pHashmap(m) && m != nullptr)
        throw SxRuntimeError("WITH-META wants a map or nil as 2nd arg, got: "
                             + rt::toString(m));
    ppush(cpIMeta(ppeek(1))->withMeta(pHashmap(m)));
    break;
}
// ... proc map-entry]
// ... proc map-entry key]
case vasm::KEY_1: {
    ppush(cpMapEntry(ppeek())->key());
    break;
}
// ... proc map-entry]
// ... proc map-entry val]
case vasm::VAL_1: {
    ppush(cpMapEntry(ppeek())->val());
    break;
}
// ... proc x]
// ... proc x n]
case vasm::COUNT_1: {
    ppush(Integer::fetch(rt::count(ppeek())));
    break;
}
// ... proc fn-or-closure]
// ... proc fn-or-closure nil]
case vasm::FN_DUMP_1: {
    if (Closure* p = pClosure(ppeek()))
        p->fn->dump();
    else
        cpFn(ppeek())->dump();
    ppush(NIL);
    break;
}
// ... proc map]
// ... proc map nil]
case vasm::PUSH_BINDINGS_1: {
    Var::pushBindings(cpHashmap(ppeek()));
    ppush(NIL);
    break;
}
// ... proc]
// ... proc nil]
case vasm::POP_BINDINGS_0: {
    Var::popBindings();
    ppush(NIL);
    break;
}
// ... proc obj]
// ... proc obj obj]
case vasm::MACROEXPAND_1_1: {
    ppush(compiler::macroExpand1(ppeek(), true));
    break;
}
// ... proc coll key]
// ... proc coll key bool]
case vasm::CONTAINS_P_2: {
    ppush(rt::contains(ppeek(1), ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc obj]
// ... proc obj obj']
case vasm::COPY_1: {
    ppush(rt::copy(ppeek()));
    break;
}
// ... proc obj]
// ... proc obj str]
case vasm::NAME_1: {
    if (Symbol* p = pSymbol(ppeek()))
        ppush(String::fetch(p->name()));
    else if (Keyword* p = pKeyword(ppeek()))
        ppush(String::fetch(p->name()));
    else if (pString(ppeek()))
        ppush(ppeek());
    else
        throw SxError("name wants a symbol, keyword, or string");
    break;
}
