/*
  compiler.cpp
  S. Edward Dolan
  Saturday, May 13 2023
*/

#include "sxp.hpp"

namespace compiler {

typedef std::vector<int> vecint_t;

#define MAX_BYTECODE_ADDRESS 0xffff

static Var* VAR_ONCE = nullptr;

enum RecurTargetType {
    RTT_LOOP,
    RTT_FN,
};

struct RecurTarget : gc {
    RecurTargetType type;
    int jumpAddr;
    int reqArgs;
    bool isRest;
    vecint_t indexes;
    RecurTarget* next;
    RecurTarget(RecurTargetType type, int jumpAddr, int reqArgs, bool isRest,
                vecint_t& indexes)
        : type(type),
          jumpAddr(jumpAddr),
          reqArgs(reqArgs),
          isRest(isRest),
          indexes(indexes) {}
};

RecurTarget* recurTarget = nullptr;

static void pushRecurTarget(RecurTarget* rt) {
    rt->next = recurTarget;
    recurTarget = rt;
}

static void popRecurTarget() {
    assert(recurTarget && "CAN'T POP EMPTY RECUR TARGET STACK");
    recurTarget = recurTarget->next;
}

struct FnIR : gc {
    Fn* fn;
    FnMethod* method;          // current method emitted to
    hashmap_t globals;         // track vars that have yet to be DEF'd
    vecobj_t freeVars;
    FnIR* parent;
    FnIR(Fn* fn, FnIR* parent)
        : fn(fn),
          method(nullptr),
          globals(),
          freeVars(),
          parent(parent) {}
};

struct LocalVar : Obj {
    LocalVar(Symbol* sym, int index, FnMethod* method)
        : sym(sym),
          index(index),
          method(method) {}
    std::string toString() {
        return rt::toString(sym);
    }
    Symbol* sym;
    int index;
    FnMethod* method;
};
DEF_CASTER(LocalVar)

struct FreeVar : Obj {
    FreeVar(int index, bool indexIsLocal)
        : index(index),
          indexIsLocal(indexIsLocal) {}
    int index;
    bool indexIsLocal;
};
DEF_CASTER(FreeVar)

// =========================================================================
//                      Intermediate Fn Representation

FnIR* thisFn = nullptr;

static void pushFn(const std::string& name) {
    thisFn = new FnIR(Fn::create(name), thisFn);
}

static int emitByte(int);
static void emitConstant(Obj*);

static Fn* popFn() {
    Fn* fn = thisFn->fn;
    vecobj_t& freeVars = thisFn->freeVars;
    if ((thisFn = thisFn->parent)) {
        emitConstant(fn);
        if (size_t n = freeVars.size()) {
            fn->nUpvals(n);
            emitByte(vasm::NEW_CLOSURE);
            emitByte(n);
            for (size_t i=0; i<n; ++i) {
                FreeVar* fv = pFreeVar(freeVars[i]);
                emitByte(fv->indexIsLocal ? 1 : 0);
                emitByte(fv->index);
            }
        }
        // TODO: meta
    }
    // fn->dump();
    return fn;
}

// =========================================================================
//                         Free Variable Management

#define MAX_FREE_VARS 256

static int registerFreeVar(FnIR* fnir, int index, bool indexIsLocal) {
    size_t nFreeVars = fnir->freeVars.size();
    if (nFreeVars == MAX_FREE_VARS) {
        std::stringstream ss;
        ss << "maximum free variables per fn (" << MAX_FREE_VARS
           << ") exceeded";
        throw SxCompilerError(ss.str());
    }
    for (size_t i=0; i<nFreeVars; ++i) {
        FreeVar* f = pFreeVar(fnir->freeVars[i]);
        if (f->index == index && f->indexIsLocal == indexIsLocal)
            return i;
        ++i;
    }
    fnir->freeVars.push_back(new (PointerFreeGC) FreeVar(index, indexIsLocal));
    return fnir->freeVars.size() - 1;
}

static int closeOver(LocalVar* loc, FnIR* fnir) {
    assert(fnir);
    assert(fnir->parent);
    if (fnir->parent->method == loc->method)
        return registerFreeVar(fnir, loc->index, true);
    return registerFreeVar(fnir, closeOver(loc, fnir->parent), false);
}

static void emitStoreFreeIdx(int i) {
    if (i < 5)
        emitByte(vasm::STORE_FREE_0 + i);
    else if (i < UINT8_MAX) {
        emitByte(vasm::STORE_FREE_B);
        emitByte(i);
    }
    else
        // TODO: should this be a runtime error or does registerFreeVar handle?
        assert(0 && "MAX_FREE_VARS exceeded");
}

static void emitLoadFreeIdx(int i) {
    if (i < 5) {
        emitByte(vasm::LOAD_FREE_0 + i);
        if (rt::toBool(VAR_ONCE->get())) { // #^{:once true} fn ...
            emitByte(vasm::LOAD_NIL);
            emitStoreFreeIdx(i);
        }
    }
    else if (i < UINT8_MAX) {
        emitByte(vasm::LOAD_FREE_B);
        emitByte(i);
        if (rt::toBool(VAR_ONCE->get())) { // same
            emitByte(vasm::LOAD_NIL);
            emitStoreFreeIdx(i);
        }
    }
    else
        // TODO: should this be a runtime error or does register free handle?
        assert(0 && "MAX_FREE_VARS exceeded");
}

// =========================================================================
//                          Local Variable Management

static ISeq* localEnv = NIL;           // ((Local* ...) ...) or NIL if empty

static void pushLocalEnv() {
    localEnv = rt::cons(localEnv, List::create());
}

static void popLocalEnv() {
    assert(localEnv != NIL);
    localEnv = rt::next(localEnv);
}

static void pushLocal(LocalVar* loc) {
    assert(localEnv != NIL && rt::first(localEnv) != NIL);
    pList(localEnv)->setHead(rt::cons(rt::first(localEnv), loc));
}

static LocalVar* registerLocal(Symbol* sym) {
    // std::cout << "registerLocal: " << sym->toString() << ' ';
    LocalVar* local = new LocalVar(sym, thisFn->method->nextLocalIdx(),
                                   thisFn->method);
    pushLocal(local);
    // std::cout << rt::toString(localEnv) << std::endl;
    return local;
}

int emitByte(int);

static void emitLoadLocalIdx(int i) {
    if (i < 5)
        emitByte(vasm::LOAD_LOCAL_0 + i);
    else if (i < UINT8_MAX) {
        emitByte(vasm::LOAD_LOCAL_B);
        emitByte(i);
    }
    else {
        emitByte(vasm::LOAD_LOCAL_S);
        emitByte(i);
        emitByte(i >> 8);
    }
}

static void emitStoreLocalIdx(int i) {
    if (i < 5)
        emitByte(vasm::STORE_LOCAL_0 + i);
    else if (i < UINT8_MAX) {
        emitByte(vasm::STORE_LOCAL_B);
        emitByte(i);
    }
    else {
        emitByte(vasm::STORE_LOCAL_S);
        emitByte(i);
        emitByte(i >> 8);
    }
}

static bool resolveLocalVar(Symbol* sym, int& index, bool& isFree) {
    // std::cout << "resolveLocalVar: " << sym->toString() << ' '
    //           << rt::toString(localEnv) << std::endl;
    for (Obj* s=rt::seq(localEnv); s!=NIL; s=rt::next(s))
        for (Obj* e=rt::seq(rt::first(s)); e!=NIL; e=rt::next(e)) {
            LocalVar* loc = pLocalVar(rt::first(e));
            if (rt::isEqualTo(sym, loc->sym)) {
                if (loc->method == thisFn->method) {
                    index = loc->index;
                    isFree = false;
                    return true;
                }
                else {
                    index = closeOver(loc, thisFn);
                    isFree = true;
                    return true;
                }
            }
        }
    return false;
}

Namespace* namespaceFor(Namespace*, Symbol*);

static Obj* resolveIn(Namespace* ns, Symbol* sym, bool allowPrivate) {
    // std::cout << "resolveIn: ns:" << ns->toString() << " sym:"
    //           << sym->toString() << ' '
    // /*<< rt::toString(localEnv)*/ << std::endl;
    if (sym->hasNS()) {
        Namespace* nsp = namespaceFor(ns, sym);
        if (!nsp)
            throw SxCompilerError("no such namespace: " + sym->nsName());
        Var* var = nsp->findInternedVar(Symbol::create(sym->name()));
        if (!var)
            throw SxCompilerError("no such var: " + sym->toString());
        else if (var->ns() != rt::currentNS() &&
                 !var->isPublic() &&
                 !allowPrivate)
            throw SxCompilerError("var: " + var->toString()
                                  + " is not public");
        return var;
    }
    else if (rt::isEqualTo(sym, rt::SYM_NS))
        return rt::VAR_NS;
    else if (rt::isEqualTo(sym, rt::SYM_IN_NS))
        return rt::VAR_IN_NS;
    if (Obj* o = ns->get(sym))
        return o;
    throw SxCompilerError("unresolved symbol: " + sym->toString());
}

static Obj* resolve(Symbol* sym) {
    return resolveIn(rt::currentNS(), sym, false);
}

// =========================================================================
//                            Symbol Resolution

Namespace* namespaceFor(Namespace* inns, Symbol* sym) {
    assert(sym->hasNS());
    Symbol* nsSym = Symbol::create(sym->nsName());
    Namespace* ns = inns->lookupAlias(nsSym);
    if (!ns)
        ns = Namespace::find(nsSym);
    return ns;
}

static Namespace* namespaceFor(Symbol* sym) {
    assert(sym->hasNS());
    return namespaceFor(rt::currentNS(), sym);
}

Symbol* resolveSymbol(Symbol* sym) {
    if (sym->hasNS()) {
        Namespace* ns = namespaceFor(sym);
        if (!ns || ns->name()->name() == sym->nsName())
            // resolves to the current namespace
            return sym;
        // resolve to other namespace
        return Symbol::create(ns->name()->name(), sym->name());
    }
    Var* var = pVar(rt::currentNS()->get(sym));
    if (!var)
        // not interned in the current ns
        return Symbol::create(rt::currentNS()->name()->name(), sym->name());
    // interned in the current ns
    return Symbol::create(var->ns()->name()->name(), var->sym()->name());
}

// =========================================================================

// return the address of the byte written
int emitByte(int byte) {
    int n = thisFn->method->nextAddress();
    if (n == MAX_BYTECODE_ADDRESS) {
        std::stringstream ss;
        ss << "max bytecode address (" << MAX_BYTECODE_ADDRESS
           << ") exceeded in fn: " << thisFn->fn->name();
        throw SxCompilerError(ss.str());
    }
    thisFn->method->appendByte(byte);
    return n;
}

// =========================================================================
// Constant pool management for the current fn

static int registerConstant(Obj* obj) {
    return thisFn->fn->appendConstant(obj);
}

static void emitLoadConstantIdx(int i) {
    if (i < 5)
        emitByte(vasm::LOAD_CONST_0 + i);
    else if (i <= UINT8_MAX) {
        emitByte(vasm::LOAD_CONST_B);
        emitByte(i);
    }
    else {
        emitByte(vasm::LOAD_CONST_S);
        emitByte((uint8_t)i);
        emitByte((uint8_t)(i >> 8));
    }
}

static void emitConstant(Obj* obj) {
    emitLoadConstantIdx(registerConstant(obj));
}

// =========================================================================

enum Ctx {
    DEFAULT,
    STATEMENT,
    EXPRESSION,
    TAIL
};

static void emit(Obj*, Ctx);

static void emitBody(Obj* s, Ctx ctx) {
    if (s == NIL)
        emitByte(vasm::LOAD_NIL);
    else {
        for (; s!=NIL; s=rt::next(s)) {
            bool moar = rt::next(s) != NIL;
            emit(rt::first(s),
                 ctx != DEFAULT &&
                 (ctx == STATEMENT || moar) ? STATEMENT : ctx);
            if (moar)
                emitByte(vasm::POP);
        }
    }
}

static void emitMethod(Obj* s, Symbol* fnName, bool fnNamed) {
    // std::cout << "emitMethod: " << rt::toString(s) << std::endl;
    enum {REQ, REST, DONE} pstate = REQ;
    Vector* v = pVector(rt::first(s));
    if (!v)
        throw SxCompilerError("FN wants a vector of parameters, got: "
                              + rt::toString(rt::first(s)));
    Symbol* sym = nullptr;
    int reqArgs = 0;
    bool isRest = false;
    vecobj_t locsyms;
    for (int i=0/*, j=0*/; i<v->count(); ++i) {
        Obj* obj = v->impl()[i];
        if (!(sym = pSymbol(obj)))
            throw SxCompilerError("FN wants a symbol in paremeter vector,"
                                  " got: " + rt::toString(obj));
        sym = pSymbol(v->impl()[i]);
        if (rt::isEqualTo(sym, rt::SYM_AMP)) {
            if (pstate == REQ)
                pstate = REST;
            else
                throw SxCompilerError("extra & found in FN parameter vector");
        }
        else {
            locsyms.push_back(sym);
            switch (pstate) {
                case REQ:
                    // inc req args
                    ++reqArgs;
                    break;
                case REST:
                    isRest = true;
                    pstate = DONE;
                    break;
                case DONE:
                    throw SxCompilerError("unexpected FN paremeter: "
                                          + rt::toString(sym));
            }
        }
    }
    if (sym && rt::isEqualTo(sym, rt::SYM_AMP))
        throw SxCompilerError("FN missing parameter after the &");
    thisFn->method = thisFn->fn->addMethod(isRest, reqArgs);
    if (fnNamed)
        registerLocal(fnName);
    else
        // don't register this fn, but leave its local slot open on the stack
        thisFn->method->nextLocalIdx(); // skip locals[0]
    vecint_t indexes;
    for (auto s : locsyms)
        indexes.push_back(registerLocal(pSymbol(s))->index);
    pushRecurTarget(new RecurTarget(RTT_FN,
                                    thisFn->method->bc().size(),
                                    thisFn->method->reqArgs(),
                                    thisFn->method->isRest(),
                                    indexes));
    emitBody(rt::next(s), TAIL);
    emitByte(vasm::RETURN);
    popRecurTarget();
}

static Fn* emitFN(Obj* s, Ctx ctx) {
    // std::cout << "emitFN:" << rt::toString(s) << std::endl;
    (void)ctx;
    Symbol* sym = nullptr;
    bool named = false;
    if ((sym = pSymbol(rt::first(s)))) {
        if (sym->hasNS())
            throw SxCompilerError("FN name cannot be ns-qualified");
        s = rt::next(s);
        named = true;
    }
    else
        sym = rt::genSym("fn__", "__AUTO__");
    if (pVector(rt::first(s)))
        s = List::create(s);    // ([...] ...) => (([...] ...) ...)
    else if (!pList(rt::first(s)))
        throw SxCompilerError("FN wants a vector of parameters or an"
                              " overloaded method, got: "
                              + rt::toString(rt::first(s)));
    pushFn(sym->name());
    for (; s!=NIL; s=rt::next(s)) {
        // pushMethodHandlers();
        pushLocalEnv();
        emitMethod(rt::first(s), sym, named);
        // transferMethodHandlers();
        popLocalEnv();
    }
    return popFn();
}

// (letfn [<sym> (name? ([<param>* <& param>?] <body>?))*] <body>?)
static void emitLETFN(Obj* s, Ctx ctx) {
    if (Vector* v = pVector(rt::first(s))) {
        if (v->count() % 2)
            throw SxCompilerError("LETFN vector missing final value");
        vecint_t indexes(v->count() / 2);
        pushLocalEnv();
        // first register all the let names, so they are visible to all fns
        for (int i=0, j=0; i<v->count(); i+=2, ++j) {
            if (Symbol* sym = pSymbol(v->nth(i))) {
                assert(sym);
                indexes[j] = registerLocal(sym)->index;
            }
            else
                throw SxCompilerError("LETFN wants a symbol in bindings,"
                                      " got: " + rt::toString(v->nth(i)));
        }
        // now all the fn forms
        for (int i=0, j=0; i<v->count(); i+=2, ++j) {
            if (List* form = pList(v->nth(i + 1))) {
                if (pSymbol(rt::first(form)))
                    emitFN(form, EXPRESSION);
                else
                    emitFN(form->cons(v->nth(i)), EXPRESSION);
                emitStoreLocalIdx(indexes[j]);
            }
            else
                throw SxCompilerError("LETFN binding value must have the"
                                      " form: (name? [<param>* <& param>?]"
                                      " <body>?), got: "
                                      + rt::toString(v->nth(i + 1)));
        }
        emitBody(rt::next(s), ctx);
        popLocalEnv();
    }
    else
        throw SxCompilerError("LETFN wants a vector as first arg");
}

static int registerVar(Var* var) {
    auto itr = thisFn->globals.find(var);
    if (itr != thisFn->globals.end())
        return pInteger(itr->second)->val();
    int index = registerConstant(var);
    thisFn->globals[var] = Integer::fetch(index);
    return index;
}

static Var* lookupVar(Symbol* sym, bool intern) {
    Var* var = NIL;
    if (sym->hasNS()) {
        Namespace* ns = namespaceFor(rt::currentNS(), sym);
        if (!ns)
            return NIL;
        Symbol* name = Symbol::create(sym->name());
        if (intern && ns == rt::currentNS())
            var = rt::currentNS()->intern(name);
        else
            var = ns->findInternedVar(name);
    }
    else if (rt::isEqualTo(sym, rt::SYM_NS))
        var = rt::VAR_NS;
    else if (rt::isEqualTo(sym, rt::SYM_IN_NS))
        var = rt::VAR_IN_NS;
    else {
        /*
          Not yet found, resolve in the current ns and intern if requested so
          subsequent compled expression can resolve the symbol.
        */
        Obj* o = rt::currentNS()->get(sym);
        if (!o) {
            if (intern)
                var = rt::currentNS()->intern(Symbol::create(sym->name()));
        }
        else if (pVar(o))
            var = pVar(o);
        else
            throw SxCompilerError("expecting a var but " + sym->toString()
                                  + " is mapped to type: " + rt::typeName(o));
    }
    if (var)
        registerVar(var);
    return var;
}

static void emitSymbol(Symbol* sym, Ctx ctx) {
    // std::cout << "emitSymbol: " << sym->toString() << std::endl;
    (void)ctx;
    int index;
    bool isFree = false;
    // local lookup, no ns
    if (!sym->hasNS())
        if (resolveLocalVar(sym, index, isFree)) {
            if (isFree)
                emitLoadFreeIdx(index);
            else
                emitLoadLocalIdx(index);
            return;
        }
    // global lookup
    Obj* o = resolve(sym);
    if (Var* var = pVar(o)) {
        if (var->isMacro()) {
            std::stringstream ss;
            ss << "can't take the value of a macro: " << rt::toString(var);
            throw SxCompilerError(ss.str());
        }
        emitLoadConstantIdx(registerVar(var));
        emitByte(vasm::VAR_GET);
        return;
    }
    else
        emitConstant(o);
}

// (quote ...)
static void emitQUOTE(Obj* form, Ctx ctx) {
    (void)ctx;
    if (rt::count(form) == 1) {
        form = rt::first(form);
        if (form == NIL) emitByte(vasm::LOAD_NIL);
        else if (form == rt::T) emitByte(vasm::LOAD_TRUE);
        else if (form == rt::F) emitByte(vasm::LOAD_FALSE);
        else emitConstant(form);
    }
    else
        throw SxCompilerError("QUOTE wants 1 arg");
}

static void emitCALL(int nArgs) {
    if (nArgs < 5)
        emitByte(vasm::CALL_0 + nArgs);
    else if (nArgs <= UINT8_MAX) {
        emitByte(vasm::CALL_B);
        emitByte(nArgs);
    }
    else {
        emitByte(vasm::CALL_S);
        emitByte((uint8_t)nArgs);
        emitByte((uint8_t)(nArgs >> 8));
    }
}

// (callable ...)
static void emitCall(Obj* form, Ctx ctx) {
    emit(rt::first(form), EXPRESSION); // push the callable onto the stack
    size_t nArgs = 0;
    for (form=rt::next(form); form!=NIL; form=rt::next(form)) {
        // and the args
        emit(rt::first(form), ctx);
        ++nArgs;
    }
    // and the CALL_N instruction
    emitCALL(nArgs);
}

// (if ...)
static void emitIF(Obj* form, Ctx ctx) {
    if (form == NIL)
        throw SxCompilerError("IF missing `test' form");
    int elseAddr = 0, endAddr = 0;
    emit(rt::first(form), ctx == DEFAULT ? ctx : EXPRESSION);
    emitByte(vasm::JUMP_IF_FALSE);
    elseAddr = emitByte(0);
    emitByte(0);
    if ((form = rt::next(form)) == NIL)
        throw SxCompilerError("IF missing '`then' form");
    emit(rt::first(form), ctx);
    emitByte(vasm::JUMP);
    endAddr = emitByte(0);
    emitByte(0);
    thisFn->method->rewrite(elseAddr);
    if ((form = rt::next(form)) != NIL) {
        if (rt::next(form) != NIL)
            throw SxCompilerError("IF wants 3 args max");
        emit(rt::first(form), ctx);
    }
    else
        emitByte(vasm::LOAD_NIL);
    thisFn->method->rewrite(endAddr);
}

static void emitHashmap(Hashmap*, Ctx);

/*
  This is used by the compiler to assign meta data and not associated with a
  user function.
*/
static void emitSET_META(Hashmap* m) {
    // std::cout << "emitSET_META:" << rt::toString(m) << std::endl;
    emitHashmap(m, EXPRESSION);
    emitByte(vasm::SET_META);
}

// (def ...)
static void emitDEF(Obj* form, Ctx ctx) {
    int n = rt::count(form);
    if (n != 1 && n != 2)
        throw SxCompilerError("DEF wants 1 or 2 args");
    if (!pSymbol(rt::first(form)))
        throw SxCompilerError("DEF wants a symbol as first arg, got: "
                              + rt::toString(rt::first(form)));
    Symbol* sym = pSymbol(rt::first(form));
    Var* var = lookupVar(sym, true); // true == intern if not found
    if (!var)
        throw SxCompilerError("DEF cannot refer to a qualified var that does"
                              " not exists");
    if (var->ns() != rt::currentNS()) {
        if (!sym->hasNS())
            var = rt::currentNS()->intern(sym);
        else
            throw SxCompilerError("DEF cannot intern into another namespace");
    }
    emitLoadConstantIdx(registerVar(var));
    if (sym->hasMeta())
        emitSET_META(sym->meta());
    if ((form = rt::next(form)))
        // (def foo THIS-EXPRESSION)
        emit(rt::first(form), ctx == DEFAULT ? ctx : EXPRESSION);
    else
        emitByte(vasm::LOAD_NIL);
    emitByte(vasm::DEF);
}

// (let ...)
static void emitLET(Obj* form, Ctx ctx) {
    if (Vector* v =  pVector(rt::first(form))) {
        if (v->count() % 2)
            throw SxCompilerError("LET vector missing final value");
        pushLocalEnv();
        for (int i=0; i<v->count(); i+=2) {
            Symbol* sym = pSymbol(v->impl()[i]);
            if (!pSymbol(sym))  // should be !sym yes?
                throw SxCompilerError("LET wants a symbol in binding vector,"
                                      " got: " + rt::toString(sym));
            if (sym->hasNS())
                throw SxCompilerError("LET binding symbols cannot be"
                                      " ns-qualified, got: "
                                      + rt::toString(sym));
            emit(v->impl()[i + 1], EXPRESSION);
            emitStoreLocalIdx(registerLocal(sym)->index);
        }
        emitBody(rt::next(form), ctx);
        popLocalEnv();
    }
    else
        throw SxCompilerError("LET wants a vector as first arg");
}

// (loop ...)
static void emitLOOP(Obj* form, Ctx ctx) {
    (void)ctx;
    if (Vector* v =  pVector(rt::first(form))) {
        if (v->count() % 2)
            throw SxCompilerError("LOOP vector missing final value");
        int count = v->count();
        vecint_t indexes;
        pushLocalEnv();
        for (int i=0; i<count; i+=2) {
            Symbol* sym = pSymbol(v->impl()[i]);
            if (!sym)
                throw SxCompilerError("LOOP wants a symbol in binding vector,"
                                      " got: " + rt::toString(sym));
            if (sym->hasNS())
                throw SxCompilerError("LOOP binding symbols cannot be"
                                      " ns-qualified, got: "
                                      + rt::toString(sym));
            emit(v->impl()[i + 1], EXPRESSION);
            LocalVar* local = registerLocal(sym);
            indexes.push_back(local->index);
            emitStoreLocalIdx(local->index);
        }
        pushRecurTarget(new RecurTarget(RTT_LOOP,
                                        thisFn->method->bc().size(),
                                        count / 2,
                                        false,
                                        indexes));
        emitBody(rt::next(form), TAIL);
        popRecurTarget();
        popLocalEnv();
    }
    else
        throw SxCompilerError("LOOP wants a vector as first arg");
}

// (recur ...)
static void emitRECUR(Obj* form, Ctx ctx) {
    if (!recurTarget)
        throw SxCompilerError("RECUR has no valid target (FN or LOOP) in"
                              " scope");
    if (ctx != TAIL)
        throw SxCompilerError("RECUR not in tail position");
    int nArgs = rt::count(form);
    if (recurTarget->type == RTT_LOOP) {
        if (nArgs != recurTarget->reqArgs) {
            std::stringstream ss;
            ss << "RECUR target (a LOOP) wants " << recurTarget->reqArgs
               << " args";
            throw SxCompilerError(ss.str());
        }
        int i = 0;
        for (; form!=NIL; form=rt::next(form), ++i)
            emit(rt::first(form), EXPRESSION);
        while (--i >= 0)
            emitStoreLocalIdx(recurTarget->indexes[i]);
    }
    else {
        // FN
        if (nArgs < recurTarget->reqArgs) {
            if (recurTarget->isRest) {
                std::stringstream ss;
                ss << "RECUR target `"  << thisFn->fn->name()
                   << "' wants at least (" << recurTarget->reqArgs
                   << ") arg(s)";
                throw SxCompilerError(ss.str());
            }
            else {
                std::stringstream ss;
                ss << "RECUR target `"  << thisFn->fn->name()
                   << "' wants (" << recurTarget->reqArgs << ") arg(s)";
                throw SxCompilerError(ss.str());
            }
        }
        else if (nArgs > recurTarget->reqArgs && !recurTarget->isRest) {
            std::stringstream ss;
            ss << "RECUR target `"  << thisFn->fn->name()
               << "' wants (" << recurTarget->reqArgs << ") arg(s)";
            throw SxCompilerError(ss.str());
        }
        int i = 0;
        // rebind req args in target FN
        for (; form!=NIL && i<recurTarget->reqArgs; form=rt::next(form), ++i) {
            emit(rt::first(form), EXPRESSION);
            emitStoreLocalIdx(recurTarget->indexes[i]);
        }
        /*
          bind the & param

          NOTE:
          RECUR does not collect the extra args into a list for the &
          param of the fn you are recurring on, which is kinda wack. You can
          pass 42 as the & arg and, on the recur, it will be bound to 42, not
          (42). This is the exact behavior of clojure.
        */ 
        if (recurTarget->isRest) {
            if (form == NIL)
                emitByte(vasm::LOAD_NIL);
            else if (rt::next(form)) {
                std::stringstream ss;
                ss << "RECUR target `"  << thisFn->fn->name()
                   << "' wants (" << recurTarget->reqArgs
                   << " or " << recurTarget->reqArgs + 1 <<  """) arg(s)";
                throw SxCompilerError(ss.str());
            }
            else 
                emit(rt::first(form), EXPRESSION);
            emitStoreLocalIdx(recurTarget->indexes[i]);
        }
    }
    emitByte(vasm::JUMP);
    emitByte(recurTarget->jumpAddr);
    emitByte(recurTarget->jumpAddr >> 8);
}

// (var x)
static void emitVAR(Obj* form, Ctx ctx) {
    (void)ctx;
    if (rt::count(form) > 1)
        throw SxCompilerError("VAR wants 1 arg");
    if (Symbol* sym = pSymbol(rt::first(form))) {
        Var* var = lookupVar(sym, false);
        if (var)
            emitLoadConstantIdx(registerVar(var));
        else
            throw SxCompilerError("unable to resolve var: "
                                  + rt::toString(sym) + ", in this scope");
    }
    else
        throw SxCompilerError("VAR wants a symbol, got: "
                              + rt::toString(rt::first(form)));
}

// (apply ...)
static void emitAPPLY(Obj* form, Ctx ctx) {
    if (rt::count(form) < 2)
        throw SxCompilerError("APPLY wants at least 2 args");
    if (ctx != DEFAULT)
        ctx = EXPRESSION;
    emit(rt::first(form), ctx);
    int nArgs = 0;
    for (form=rt::next(form); form!=NIL; form=rt::next(form), ++nArgs)
        emit(rt::first(form), ctx);
    emitConstant(Integer::fetch(nArgs));
    emitByte(vasm::APPLY);
}

// (throw <form>?)
static void emitTHROW(Obj* form, Ctx ctx) {
    (void)ctx;
    static Obj* err = rt::sxpNS()->get(Symbol::create("SxError"));
    int n = rt::count(form);
    if (n == 0)
        // (throw) no args, should emit the exact same instance every time
        emitConstant(err);
    else if (n == 1)
        emit(rt::first(form), EXPRESSION);
    else
        throw SxCompilerError("THROW wants 1 arg");
    emitByte(vasm::THROW);
}

static void checkTry(Obj* s, Vector* tryExprs, Vector* catches,
                     Obj** finallyExpr) {
    for (; s; s=rt::next(s)) {
        Obj* e = rt::first(s), *op = pList(e) ? rt::first(e) : NIL;
        if (!rt::isEqualTo(op, rt::SYM_CATCH)
            && !rt::isEqualTo(op, rt::SYM_FINALLY)) {
            if (!rt::isEmpty(catches))
                throw SxCompilerError("only CATCH and FINALLY may follow a"
                                      " CATCH in a TRY form.");
            tryExprs->conj(e);
        }
        else {
            // (CATCH sym sym <expr>*)
            if (rt::isEqualTo(op, rt::SYM_CATCH)) {
                Obj* catchRest = rt::next(e);
                if (rt::count(catchRest) < 2)
                    throw SxCompilerError("CATCH wants two symbols, minimum");
                // check 1st arg
                Obj* sym1 = rt::first(catchRest);
                if (!pSymbol(sym1))
                    throw SxCompilerError("CATCH wants a symbol as first arg"
                                          " got: " + rt::toString(sym1));
                Obj* t = resolve(pSymbol(sym1));
                if (!pSxError(t))
                    throw SxCompilerError("CATCH wants an SxError or subclass-"
                                          "of as first arg, got type: "
                                          + rt::typeName(t));
                // check 2nd arg
                Obj* sym2 = rt::second(catchRest);
                if (!pSymbol(sym2))
                    throw SxCompilerError("CATCH wants a symbol as second arg"
                                          " got: " + rt::toString(sym2));
                if (pSymbol(sym2)->hasNS())
                    throw SxCompilerError("CATCH second symbol cannot be"
                                          " ns-qualified, got: "
                                          + sym2->toString());
                catches->conj(e);
            }
            else if (rt::next(s) != NIL)
                throw SxCompilerError("FINALLY must occur once, at the tail"
                                      " of its enclosing TRY");
            else
                *finallyExpr = e;
        }
    }
}

/*
  (try <expr>* <catch>* <finally>?)

  The following code ...

  (try
  (catch SxArithmeticError e)
  (catch SxIOError e)
  (finally))

  ... will be emitted as:

  bc: 0000 LOAD_NIL
      0001 LOAD_NIL
      0002 POP
      0003 JUMP................0019 (25)
      0006 STORE_LOCAL_1
      0007 LOAD_NIL
      0008 LOAD_NIL
      0009 POP
      000a JUMP................0019 (25)
      000d STORE_LOCAL_2
      000e LOAD_NIL
      000f LOAD_NIL
      0010 POP
      0011 JUMP................0019 (25)
      0014 STORE_LOCAL_3
      0015 LOAD_NIL
      0016 POP
      0017 LOAD_LOCAL_3
      0018 RETHROW
      0019 RETURN
  handlers: psa=0000 pea=0001 hsa=0006 SxArithmeticError
            psa=0000 pea=0001 hsa=000d SxIOError
            psa=0000 pea=0001 hsa=0014 SxAny
            psa=0006 pea=0008 hsa=0014 SxAny
            psa=000d pea=000f hsa=0014 SxAny

  TODO: Possibly use JSR & RET opcodes (6502 local subroutines) to remove the
  duplicate FINALLY code. As is, the FINALLY code is emitted once, just
  after the proteced code, once per CATCH, and once more in the
  FINALLY. There usually isn't much code in a finally but still, it
  would be a cool feature of the VM.
*/

// original working emitter
static void emitTRY(Obj* form, Ctx ctx) {
    // (TRY tryExprs catch* finally?)
    Vector* tryExprs = Vector::create();
    Vector* catches = Vector::create();
    Obj* finallyExpr = NIL;
    /*
      First, collect all the expressions in the TRY form doing a bit of error
      handling.
    */
    checkTry(form, tryExprs, catches, &finallyExpr);
    /*
      The TRY expressions, CATCH forms, and FINALLY form should now be in the
      correct order, Sort it all out...

      First, emit the TRY protected code, keeping track of the code's address
      range for the entry into the current method's handlers.
    */
    int tryPCodeStart = thisFn->method->nextAddress();
    emitBody(rt::seq(tryExprs), ctx);
    // the next instr AFTER the protected code
    int tryPCodeEnd = thisFn->method->nextAddress();
    // at this point, the result of the try form is on the stack
    if (finallyExpr) {
        /*
          Next, emit a copy of the finally code which is for side-effect only,
          hence the trailing pop instruction. This code gets executed if there
          is no error in the protected code block.
        */
        emitBody(rt::next(finallyExpr), ctx);
        emitByte(vasm::POP);
    }
    /*
      Next, maybe emit code to jump over a CATCH and/or FINALLY, tryEndAddr
      may get re-written below.
    */
    int tryEndAddr = 0;
    bool gotCatch = !catches->isEmpty(), gotFinally = finallyExpr;
    if (gotCatch || gotFinally) {
        emitByte(vasm::JUMP);
        tryEndAddr = emitByte(thisFn->method->nextAddress() + 2);
        emitByte(0);
    }
    vecint_t rwaddr;
    Vector* anys = NIL;
    if (gotCatch) {
        if (!gotFinally) {
            Obj* c = NIL;
            for (int i=0; i<catches->count(); ++i) {
                pushLocalEnv();
                c = catches->nth(i);
                SxError* e = pSxError(resolve(pSymbol(rt::second(c))));
                int handlerAddr = thisFn->method->nextAddress();
                emitStoreLocalIdx(registerLocal(pSymbol(rt::third(c)))->index);
                emitBody(rt::next(rt::next(rt::next(c))), ctx);
                if (i+1 < catches->count()) {
                    emitByte(vasm::JUMP);
                    rwaddr.push_back(emitByte(0));
                    emitByte(0);
                }
                thisFn->method->addHandler(tryPCodeStart, tryPCodeEnd,
                                           handlerAddr, e);
                popLocalEnv();
            }
            for (auto a : rwaddr)
                thisFn->method->rewrite(a);
            thisFn->method->rewrite(tryEndAddr);
        }
        else {
            Obj* c = NIL;
            anys = Vector::create();
            for (int i=0; i<catches->count(); ++i) {
                // for each catch...
                pushLocalEnv();  // this CATCH form scope
                c = catches->nth(i);
                SxError* e = pSxError(resolve(pSymbol(rt::second(c))));
                int handlerAddr = thisFn->method->nextAddress();
                // (catch SxError e) ; store the e locally
                emitStoreLocalIdx(registerLocal(pSymbol(rt::third(c)))->index);
                // emit the catch body
                emitBody(rt::next(rt::next(rt::next(c))), ctx);
                anys->conj(Integer::fetch(handlerAddr));
                anys->conj(Integer::fetch(thisFn->method->nextAddress()));
                /*
                  Emit the finally body for side-effect only. This is NOT
                  included in the SxAny handler's `protected' code.
                */
                emitBody(rt::next(finallyExpr), ctx);
                emitByte(vasm::POP);
                /*
                  Jump to the end of the TRY block. There is a finally so
                  every catch will have a jump after it.
                */
                emitByte(vasm::JUMP);
                rwaddr.push_back(emitByte(0));
                emitByte(0);
                thisFn->method->addHandler(tryPCodeStart, tryPCodeEnd,
                                           handlerAddr, e);
                popLocalEnv();
            }
        }
    }
    int finallyAddr = -1;
    if (gotFinally) {
        /*
          Emit the actually FINALLY clause of this TRY. This code block is
          only executed when an exception occurs in the protected TRY code and
          there is no applicable CATCH or, if an exception happens with an
          accompanying CATCH.
        */
        LocalVar* any = registerLocal(rt::genSym("SxAny__", "__AUTO__"));
        finallyAddr = thisFn->method->nextAddress();
        emitStoreLocalIdx(any->index);
        emitBody(rt::next(finallyExpr), ctx);
        emitByte(vasm::POP);
        emitLoadLocalIdx(any->index);
        emitByte(vasm::RETHROW);
        for (auto a : rwaddr)
            thisFn->method->rewrite(a);
        thisFn->method->rewrite(tryEndAddr);
        thisFn->method->addHandler(tryPCodeStart, tryPCodeEnd,
                                   finallyAddr,
                                   new (PointerFreeGC) SxAny());
    }
    if (anys)
        for (int i=0; i<anys->count(); i+=2) {
            int s = pInteger(anys->nth(i))->val();
            int e = pInteger(anys->nth(i+1))->val();
            thisFn->method->addHandler(s, e, finallyAddr,
                                       new (PointerFreeGC) SxAny());
        }
}

// (set! sym val)
static void emitSET_BANG(Obj* form, Ctx ctx) {
    (void)ctx;
    if (rt::count(form) != 2)
        throw SxCompilerError("SET! wants 2 args, a symbol and a value");
    if (Symbol* sym = pSymbol(rt::first(form))) {
        bool isFree;
        int index = 0;
        if (!sym->hasNS()) {
            // local?
            if (resolveLocalVar(sym, index, isFree)) {
                emit(rt::second(form), EXPRESSION);
                emitByte(vasm::DUP);
                if (isFree)
                    emitStoreFreeIdx(index);
                else
                    emitStoreLocalIdx(index);
                return;
            }
        }
        Obj* o = resolve(sym);
        if (Var* v = pVar(o)) {
            emit(rt::second(form), EXPRESSION);
            emitLoadConstantIdx(registerVar(v));
            emitByte(vasm::VAR_SET);
            return;
        }
        else if (o)
            // sym is bound to a `constant' object
            throw SxError("cannot rebind constant: " + sym->toString());
        else
            throw SxCompilerError("unresolved symbol: " + sym->toString());
    }
    else
        throw SxCompilerError("SET wants a symbol as first arg, got: "
                              + rt::toString(rt::first(form)));
}

static Var* getMacro(Symbol* sym) {
    int index;
    bool isFree;
    if (!sym->hasNS())
        if (resolveLocalVar(sym, index, isFree))
            return nullptr;
    Obj* o = resolve(sym);
    if (Var* v = pVar(o)) {
        // TODO: public var?
        if (v->isMacro())
            return v;
    }
    return nullptr;
}

static void init();

Obj* macroExpand1(Obj* form, bool initalize) {
    if (initalize)
        init();
    if (pList(form) && (pSymbol(rt::first(form)))) {
        // Is this a macro call?...
        Symbol* sym = pSymbol(rt::first(form));
        if (rt::isSpecial(sym))
            return form;        // ...no
        Var* var = getMacro(sym);
        if (!var)
            return form;        // ...no
        // ...yes
        pushFn(rt::genName("MACRO_EXPANDER_THUNK__", "__AUTO__"));
        thisFn->method = thisFn->fn->addMethod(false, 0);
        thisFn->method->nextLocalIdx();
        emitConstant(var->get());
        int nArgs = 0;
        for (ISeq* s=rt::next(form); s!=NIL; s=rt::next(s), ++nArgs)
            emitConstant(rt::first(s));
        emitCALL(nArgs);
        emitByte(vasm::RETURN);
        Fn* fn = thisFn->fn;
        // fn->dump();
        VM vm;
        Obj* x = vm.run(fn);
        thisFn = thisFn->parent;
        return x;
    }
    return form;
}

static Obj* macroExpand(Obj* form) {
    Obj* x = macroExpand1(form);
    if (x != form)
        return macroExpand(x);
    return x;
}

static void emitList(List* list, Ctx ctx) {
    if (Symbol* sym = pSymbol(rt::first(list))) {
        if (rt::isEqualTo(sym, rt::SYM_QUOTE))
            emitQUOTE(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_DO))
            emitBody(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_IF))
            emitIF(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_FN)) {
            DynScope ds(VAR_ONCE,
                        rt::toBool(rt::get(rt::meta(sym), rt::KW_ONCE))
                        ? rt::T : rt::F);
            emitFN(rt::next(list), ctx);
        }
        else if (rt::isEqualTo(sym, rt::SYM_DEF))
            emitDEF(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_LET))
            emitLET(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_LETFN))
            emitLETFN(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_LOOP))
            emitLOOP(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_RECUR))
            emitRECUR(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_VAR))
            emitVAR(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_APPLY))
            emitAPPLY(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_THROW))
            emitTHROW(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_TRY))
            emitTRY(rt::next(list), ctx);
        else if (rt::isEqualTo(sym, rt::SYM_SET_BANG))
            emitSET_BANG(rt::next(list), ctx);
        else
            emitCall(list, ctx);
    }
    else
        emitCall(list, ctx);
}

// [...]
static void emitVector(Vector* v, Ctx ctx) {
    (void)ctx;
    if (v->count() == 0)
        emitByte(vasm::LOAD_EMPTY_VECTOR);
    else {
        for (auto e : v->impl())
            emit(e, EXPRESSION);
        emitConstant(Integer::fetch(v->count()));
        emitByte(vasm::NEW_VECTOR);
    }
}

// {...}
static void emitHashmap(Hashmap* m, Ctx ctx) {
    (void)ctx;
    if (m->count() == 0)
        emitByte(vasm::LOAD_EMPTY_HASHMAP);
    else {
        for (auto e : m->impl()) {
            emit(e.first, EXPRESSION);
            emit(e.second, EXPRESSION);
        }
        emitConstant(Integer::fetch(m->count())); // # of kv pairs
        emitByte(vasm::NEW_HASHMAP);
    }
}

// {...}
static void emitHashset(Hashset* m, Ctx ctx) {
    (void)ctx;
    if (m->count() == 0)
        emitByte(vasm::LOAD_EMPTY_HASHSET);
    else {
        for (auto e : m->impl())
            emit(e, EXPRESSION);
        emitConstant(Integer::fetch(m->count()));
        emitByte(vasm::NEW_HASHSET);
    }
}

static void emit(Obj* obj, Ctx ctx) {
    if (obj == NIL) emitByte(vasm::LOAD_NIL);
    else if (obj == rt::T) emitByte(vasm::LOAD_TRUE);
    else if (obj == rt::F) emitByte(vasm::LOAD_FALSE);
    else if (pSymbol(obj)) emitSymbol(pSymbol(obj), ctx);
    else if (List* lst = pList(obj)) {
        if (lst->isEmpty())
            emitByte(vasm::LOAD_EMPTY_LIST);
        else {
            Obj* x = macroExpand(obj);
            if (pList(x))
                emitList(pList(x), ctx);
            else
                emit(x, ctx);
        }
    }
    else if (Vector* v = pVector(obj))
        emitVector(v, ctx);
    else if (Hashmap* m = pHashmap(obj))
        emitHashmap(m, ctx);
    else if (Hashset* m = pHashset(obj))
        emitHashset(m, ctx);
    else
        emitConstant(obj);
}

// =========================================================================

static void init() {
    if (!VAR_ONCE) {
        VAR_ONCE = Var::create();
        VAR_ONCE->setDynamic();
    }
    thisFn = nullptr;
    localEnv = nullptr;
    recurTarget = nullptr;
}

Fn* compile(Obj* form) {
    init();
    Fn* f = emitFN(List::create(rt::genSym("COMPILER_THUNK__", "__AUTO__"),
                                Vector::create(), form),
                   DEFAULT);
    // f->dump();
    return f;
}

} // end namespace compiler
