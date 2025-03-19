/*
  instr_8.cpp
  S. Edward Dolan
  Tuesday, May 23 2023
*/

// ... x]
// ...]
case vasm::POP: {
    ppop();
    break;
}
// ... x]
// ... x x]
case vasm::DUP: {
    ppush(ppeek());
    break;
}
// -------------------------------------------------------------------------
// ...]
// ... nil]
case vasm::LOAD_NIL: {
    ppush(NIL);
    break;
}
// ...]
// ... true]
case vasm::LOAD_TRUE: {
    ppush(rt::T);
    break;
}
// ...]
// ... false]
case vasm::LOAD_FALSE: {
    ppush(rt::F);
    break;
}
// -------------------------------------------------------------------------
// ...]
// ... ()]
case vasm::LOAD_EMPTY_LIST: {
    ppush(List::create());
    break;
}
// ...]
// ... []]
case vasm::LOAD_EMPTY_VECTOR: {
    ppush(Vector::create());
    break;
}
// ...]
// ... {}]
case vasm::LOAD_EMPTY_HASHMAP: {
    ppush(Hashmap::create());
    break;
}
// ...]
// ... #{}]
case vasm::LOAD_EMPTY_HASHSET: {
    ppush(Hashset::create());
    break;
}
// -------------------------------------------------------------------------
// ... a1 a2 ... aN N]
// ... [a1 a2 ... aN]]
case vasm::NEW_VECTOR: {
    int n = pInteger(ppop())->val();
    vecobj_t v(n);
    while (n--)
        v[n] = ppop();
    ppush(Vector::create(v));
    break;
}
// ... k1 v1 k2 v2 ... kN vN N]
// ... {k1 v2 kN vN ... k2 v2}           unordered
case vasm::NEW_HASHMAP: {
    int n = pInteger(ppop())->val();
    hashmap_t m;
    while (n--) {
        Obj* val = ppop();
        m[ppop()] = val;
    }
    ppush(Hashmap::create(m));
    break;
}
// TODO: Is this instruction needed? It's not emitted by the compiler.
// ... a1 a2 ... aN N]
// ... (a1 a2 ... aN)]
case vasm::NEW_LIST: {
    int n = pInteger(ppop())->val();
    assert(n);                 // this instruction does not create empty lists
    ISeq* s = List::create(ppop());
    while (--n)
        s = s->cons(ppop());
    ppush(s);
    break;
}
// ... x y z N]
// ... #{x y z}]
case vasm::NEW_HASHSET: {
    int n = pInteger(ppop())->val();
    vecobj_t v(n);
    while (n--)
        v[n] = ppop();
    ppush(Hashset::create(v));
    break;
}
// ...]
// ...]                     unchanged
case vasm::JUMP: {
    pc = READ_U16();
    break;
}
// ... x]
// ...]
case vasm::JUMP_IF_FALSE: {
    if (!rt::toBool(ppop()))
        pc = READ_U16();
    else
        pc += 2;
    break;
}
// -------------------------------------------------------------------------
// ...]
// ... x]
case vasm::LOAD_CONST_0: {
    ppush(curFrame->cp[0]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_1: {
    ppush(curFrame->cp[1]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_2: {
    ppush(curFrame->cp[2]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_3: {
    ppush(curFrame->cp[3]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_4: {
    ppush(curFrame->cp[4]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_B: {
    ppush(curFrame->cp[curFrame->bc[pc++]]);
    break;
}
// ...]
// ... x]
case vasm::LOAD_CONST_S: {
    ppush(curFrame->cp[READ_U16()]);
    pc += 2;
    break;
}
// -------------------------------------------------------------------------
// ...]
// ... x]
case vasm::LOAD_LOCAL_0: {
    ppush(*curFrame->locals);
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_1: {
    ppush(*(curFrame->locals + 1));
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_2: {
    ppush(*(curFrame->locals + 2));
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_3: {
    ppush(*(curFrame->locals + 3));
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_4: {
    ppush(*(curFrame->locals + 4));
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_B: {
    ppush(*(curFrame->locals + curFrame->bc[pc++]));
    break;
}
// ...]
// ... x]
case vasm::LOAD_LOCAL_S: {
    ppush(*(curFrame->locals + READ_U16()));
    pc += 2;
    break;
}
// -------------------------------------------------------------------------
// ... x]
// ...]
case vasm::STORE_LOCAL_0: {
    *(curFrame->locals) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_1: {
    *(curFrame->locals + 1) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_2: {
    *(curFrame->locals + 2) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_3: {
    *(curFrame->locals + 3) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_4: {
    *(curFrame->locals + 4) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_B: {
    *(curFrame->locals + curFrame->bc[pc++]) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_LOCAL_S: {
    *(curFrame->locals + READ_U16()) = ppop();
    pc += 2;
    break;
}
// -------------------------------------------------------------------------
// ...]
// ... x]
case vasm::LOAD_FREE_0: {
    ppush(*curFrame->closure->upvals[0]->addr);
    break;
}
// ...]
// ... x]
case vasm::LOAD_FREE_1: {
    ppush(*curFrame->closure->upvals[1]->addr);
    break;
}
// ...]
// ... x]
case vasm::LOAD_FREE_2: {
    ppush(*curFrame->closure->upvals[2]->addr);
    break;
}
// ...]
// ... x]
case vasm::LOAD_FREE_3: {
    ppush(*curFrame->closure->upvals[3]->addr);
    break;
}
// ...]
// ... x]
case vasm::LOAD_FREE_4: {
    ppush(*curFrame->closure->upvals[4]->addr);
    break;
}
// ...]
// ... x]
case vasm::LOAD_FREE_B: {
    ppush(*curFrame->closure->upvals[curFrame->bc[pc++]]->addr);
    break;
}
// -------------------------------------------------------------------------
// ... x]
// ...]
case vasm::STORE_FREE_0: {
    *(curFrame->closure->upvals[0]->addr) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_FREE_1: {
    *(curFrame->closure->upvals[1]->addr) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_FREE_2: {
    *(curFrame->closure->upvals[2]->addr) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_FREE_3: {
    *(curFrame->closure->upvals[3]->addr) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_FREE_4: {
    *(curFrame->closure->upvals[4]->addr) = ppop();
    break;
}
// ... x]
// ...]
case vasm::STORE_FREE_B: {
    *(curFrame->closure->upvals[curFrame->bc[pc++]]->addr) = ppop();
    break;
}
// -------------------------------------------------------------------------
// ... Fn]
// ... Closure]
case vasm::NEW_CLOSURE: {
    Closure* c = Closure::create(cpFn(ppop()));
    const vecu8_t& bc = curFrame->method->bc();
    int n = bc[pc++]; // n closed-over vars
    for (int i=0; i<n; ++i) {
        uint8_t isLocal = bc[pc++];
        uint8_t index = bc[pc++];
        if (isLocal)
            c->upvals[i] = captureUpval(index);
        else
            c->upvals[i] = curFrame->closure->upvals[index];
    }
    ppush(c);
    break;
}
// -------------------------------------------------------------------------
// ... var val]
// ... var']
case vasm::DEF: {
    Obj* val = ppop();
    pVar(pstack.back())->setRoot(val, false); // <- don't reset macro flag
    break;
}
// ... var]
// ... x]
case vasm::VAR_GET: {
    ppush(pVar(ppop())->get());
    break;
}
// -------------------------------------------------------------------------
// ... callable]
// ... callable]
case vasm::CALL_0: {
    doCall(ppeek(), 0);
    break;
}
// ... callable a1]
// ... callable a1]
case vasm::CALL_1: {
    doCall(ppeek(1), 1);
    break;
}
// ... callable a1 a2]
// ... callable a1 a2]
case vasm::CALL_2: {
    doCall(ppeek(2), 2);
    break;
}
// ... callable a1 a2 a3]
// ... callable a1 a2 a3]
case vasm::CALL_3: {
    doCall(ppeek(3), 3);
    break;
}
// ... callable a1 a2 a3 a4]
// ... callable a1 a2 a3 a4]
case vasm::CALL_4: {
    doCall(ppeek(4), 4);
    break;
}

// ... callable a1 a2 a3 a4 ... aFF]
// ... callable a1 a2 a3 a4 ... aFF]
case vasm::CALL_B: {
    int nArgs = curFrame->bc[pc++];
    doCall(ppeek(nArgs), nArgs);
    break;
}
// ... callable a1 a2 a3 a4 ... aFFFF]
// ... callable a1 a2 a3 a4 ... aFFFF]
case vasm::CALL_S: {
    int nArgs = READ_U16();
    pc += 2;
    doCall(ppeek(nArgs), nArgs);
    break;
}
// ... proc arg*]
// ... proc arg*]
case vasm::CALL_PROC: {
    uint16_t id = READ_U16();
    pc += 2;
    GOTO(id);
    break;
}
// ... cfn arg*]
// ... cfn arg* result]
case vasm::CALL_CFN: {
    CFn* cfn = cpCFn(pstack[curFrame->fnIndex]);
    ppush(cfn->cfn()(&pstack[curFrame->fnIndex]));
    break;
}
// -------------------------------------------------------------------------
// ... x]
// ...]
case vasm::RETURN: {
    if (fpop())
        return ppop();
    break;
}
// ... var map]
// ... var]
case vasm::SET_META: {
    Obj* m = ppop();
    rt::withMeta(ppeek(), cpHashmap(m));
    break;
}
// ... callable a1 a2 ... aN-1 (e1 e2 ... eM) N]
// ... callable a2 a2 ... aN e1 e2 ... eM]
case vasm::APPLY: {
    int nArgs = pInteger(ppop())->val() - 1;
    for (ISeq* s=rt::seq(ppop()); s!=NIL; s=rt::next(s), ++nArgs)
        ppush(rt::first(s));    // unpack the tail seq
    doCall(cpFn(ppeek(nArgs)), nArgs);
    break;
}
// ... x y]
// ... y x]
case vasm::SWAP: {
    size_t i = pstack.size() - 2;
    std::iter_swap(pstack.begin() + i, pstack.begin() + i + 1);
    break;
}
// ... x y z]
// ... y x z]
case vasm::SWAP2: {
    size_t i = pstack.size() - 3;
    std::iter_swap(pstack.begin() + i, pstack.begin() + i + 1);
    break;
}
// ... SxError-or-string]
// ...]
case vasm::THROW: {
    Obj* obj = ppop();
    if (String* p = pString(obj))
        throw SxError(p->val());
    else if (SxError* p = pSxError(obj))
        throw *p;
    else {
        std::stringstream ss;
        ss << "throw wants an SxError instance or a string, got: "
           << rt::typeName(obj);
        throw SxRuntimeError(ss.str());
    }
    break;
}
// ... SxError]
// ...]
case vasm::RETHROW: {
    throw *cpSxError(ppop());
    break;
}
// ... val var]
// ... val]
case vasm::VAR_SET: {
    cpVar(ppop())->set(ppeek());
    break;
}
// ...]
// ...]
case vasm::JSR: {
    uint16_t addr = READ_U16();
    jpush(pc + 2);    // resume address after the JSR instruction
    pc = addr;
    break;
}
// ...]
// ...]
case vasm::RET: {
    pc = jpop();      // resume address after the JSR instruction
    break;
}
