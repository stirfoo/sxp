/*
  rt.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

namespace rt {

bool vmTrace;            // used every VM instance

Bool* T = nullptr;
Bool* F = nullptr;

size_t uniqueID = 0;

SysInStream* SYS_IN = nullptr;
SysOutStream* SYS_OUT = nullptr;
SysOutStream* SYS_ERR = nullptr;

Obj* UNBOUND = nullptr;
Obj* SENTINEL = nullptr;
INumber* INT_ZERO = nullptr;
INumber* INT_ONE = nullptr;

Symbol* SYM_VAR = nullptr;
Symbol* SYM_UNQUOTE = nullptr;
Symbol* SYM_UNQUOTE_SPLICING = nullptr;
Symbol* SYM_LIST = nullptr;
Symbol* SYM_CONCAT = nullptr;
Symbol* SYM_SEQ = nullptr;
Symbol* SYM_VECTOR = nullptr;
Symbol* SYM_HASHMAP = nullptr;
Symbol* SYM_HASHSET = nullptr;
Symbol* SYM_NS = nullptr;
Symbol* SYM_IN_NS = nullptr;
// special symbols
Symbol* SYM_QUOTE = nullptr;
Symbol* SYM_DO = nullptr;
Symbol* SYM_IF = nullptr;
Symbol* SYM_DEF = nullptr;
Symbol* SYM_SET_BANG = nullptr;
Symbol* SYM_LET = nullptr;
Symbol* SYM_FN = nullptr;
Symbol* SYM_LOOP = nullptr;
Symbol* SYM_RECUR = nullptr;
Symbol* SYM_APPLY = nullptr;
Symbol* SYM_LETFN = nullptr;
Symbol* SYM_TRY = nullptr;
Symbol* SYM_CATCH = nullptr;
Symbol* SYM_FINALLY = nullptr;
Symbol* SYM_THROW = nullptr;
Symbol* SYM_AMP = nullptr;

Namespace* NS_SXP = nullptr;

Var* VAR_CUR_NS = nullptr;
Var* VAR_PRINT_READABLY = nullptr;
Var* VAR_FLUSH_ON_NEWLINE = nullptr;
Var* VAR_IN = nullptr;
Var* VAR_OUT = nullptr;
Var* VAR_ERR = nullptr;
Var* VAR_NS = nullptr;
Var* VAR_IN_NS = nullptr;

Keyword* KW_MACRO = nullptr;
Keyword* KW_PRIVATE = nullptr;
Keyword* KW_TAG = nullptr;
Keyword* KW_DOC = nullptr;
Keyword* KW_PARAMS = nullptr;
Keyword* KW_ONCE = nullptr;
Keyword* KW_DYNAMIC = nullptr;
Keyword* KW_APP = nullptr;
Keyword* KW_BINARY = nullptr;
Keyword* KW_IN = nullptr;
Keyword* KW_OUT = nullptr;
Keyword* KW_TRUNC = nullptr;
Keyword* KW_ATE = nullptr;

Hashmap* DEFAULT_IMPORTS = nullptr;

static VM* curVM = nullptr;
std::vector<VM*> vmStack;
void pushVM(VM* vm) {vmStack.push_back(curVM = vm);}
void popVM() {curVM = vmStack.back(); vmStack.pop_back();}
VM* currentVM() {assert(curVM); return curVM;}

void gc(void) {
    size_t i = GC_get_free_bytes(), j = 0;
    while (true) {
        GC_gcollect();
        if ((j = GC_get_free_bytes()) == i)
            return;
        i = j;
    }
}

static std::string commify(long x) {
    std::stringstream ss;
    ss << x;
    std::string s = ss.str();
    for (int i=s.size()-1, j=0; i>=0; --i, ++j)
        if (j && !(j % 3) && s[i] != '-' && s[i] != '+')
            s.insert(i + 1, 1, ',');
    return s;
}

static void printGCInfo(void) {
    puts("\nGC Info =============================================");
    printf("      heap size: %s\n"
           "     free bytes: %s\n"
           "  alloc post gc: %s\n"
           "bytes allocated: %s\n",
           commify(GC_get_heap_size()).c_str(),
           commify(GC_get_free_bytes()).c_str(),
           commify(GC_get_bytes_since_gc()).c_str(),
           commify(GC_get_total_bytes()).c_str());
}

void shutdown() {
    String::shutdown();
    Keyword::shutdown();
    Character::shutdown();
    Integer::shutdown();
    // Bool::shutdown();
    Namespace::shutdown();
    specials.clear();
    std::cout << "\nVM instructions executed: "
              << commify(VM::nInstructions());
    printGCInfo();
}

static void initSpecials() {
    specials.insert(SYM_QUOTE = Symbol::create("quote"));
    specials.insert(SYM_DO = Symbol::create("do"));
    specials.insert(SYM_IF = Symbol::create("if"));
    specials.insert(SYM_DEF = Symbol::create("def"));
    specials.insert(SYM_SET_BANG = Symbol::create("set!"));
    specials.insert(SYM_LET = Symbol::create("let*"));
    specials.insert(SYM_LETFN = Symbol::create("letfn"));
    specials.insert(SYM_FN = Symbol::create("fn"));
    specials.insert(SYM_LOOP = Symbol::create("loop"));
    specials.insert(SYM_RECUR = Symbol::create("recur"));
    specials.insert(SYM_APPLY = Symbol::create("apply"));
    specials.insert(SYM_LETFN = Symbol::create("letfn"));
    specials.insert(SYM_VAR = Symbol::create("var"));
    specials.insert(SYM_TRY = Symbol::create("try"));
    specials.insert(SYM_CATCH = Symbol::create("catch"));
    specials.insert(SYM_FINALLY = Symbol::create("finally"));
    specials.insert(SYM_THROW = Symbol::create("throw"));
    specials.insert(SYM_AMP = Symbol::create("&"));
}

static void initDefaultImports() {
    DEFAULT_IMPORTS = Hashmap::create ({
            Symbol::create("SxError"),
            new (PointerFreeGC) SxError(),
            Symbol::create("SxIOError"),
            new (PointerFreeGC) SxIOError(),
            Symbol::create("SxSortError"),
            new (PointerFreeGC) SxSortError(),
            Symbol::create("SxCastError"),
            new (PointerFreeGC) SxCastError(),
            Symbol::create("SxSyntaxError"),
            new (PointerFreeGC) SxSyntaxError(),
            Symbol::create("SxReaderError"),
            new (PointerFreeGC) SxReaderError(),
            Symbol::create("SxRuntimeError"),
            new (PointerFreeGC) SxRuntimeError(),
            Symbol::create("SxCompilerError"),
            new (PointerFreeGC) SxCompilerError(),
            Symbol::create("SxArithmeticError"),
            new (PointerFreeGC) SxArithmeticError(),
            Symbol::create("SxOutOfBoundsError"),
            new (PointerFreeGC) SxOutOfBoundsError(),
            Symbol::create("SxNotImplementedError"),
            new (PointerFreeGC) SxNotImplementedError(),
            Symbol::create("SxIllegalArgumentError"),
            new (PointerFreeGC) SxIllegalArgumentError(),
        });
}

void init() {
    vmTrace = false;            // ...maybe a cl option
    T = Bool::create(true);
    F = Bool::create(false);
    Integer::init();
    Character::init();
    List::init();
    Namespace::init();
    initSpecials();
    initDefaultImports();
    // 
    SYS_IN = SysInStream::create("stdin", std::cin);
    SYS_OUT = SysOutStream::create("stdout", std::cout);
    SYS_ERR = SysOutStream::create("stderr", std::cerr);
    // 
    UNBOUND = Obj::create();
    SENTINEL = Obj::create();
    INT_ZERO = Integer::fetch(0);
    INT_ONE = Integer::fetch(1);
    //
    SYM_UNQUOTE = Symbol::create("sxp/unquote");
    SYM_UNQUOTE_SPLICING = Symbol::create("sxp/unquote_splicing");
    SYM_LIST = Symbol::create("sxp/list");
    SYM_CONCAT = Symbol::create("sxp/concat");
    SYM_SEQ = Symbol::create("sxp/seq");
    SYM_VECTOR = Symbol::create("sxp/vector");
    SYM_HASHMAP = Symbol::create("sxp/hashmap");
    SYM_HASHSET = Symbol::create("sxp/hashset");
    SYM_NS = Symbol::create("ns");    // no namespace
    SYM_IN_NS = Symbol::create("in-ns"); // ditto
    // 
    NS_SXP = Namespace::fetch(Symbol::create("sxp"));
    // 
    KW_DYNAMIC = Keyword::fetch("dynamic");
    KW_MACRO = Keyword::fetch("macro");
    KW_PRIVATE = Keyword::fetch("private");
    KW_TAG = Keyword::fetch("tag");
    KW_DOC = Keyword::fetch("doc");
    KW_PARAMS = Keyword::fetch("params");
    KW_ONCE = Keyword::fetch("once");
    KW_APP = Keyword::fetch("app");
    KW_BINARY = Keyword::fetch("binary");
    KW_IN = Keyword::fetch("in");
    KW_OUT = Keyword::fetch("out");
    KW_TRUNC = Keyword::fetch("trunc");
    KW_ATE = Keyword::fetch("ate");
    // 
    VAR_IN = Var::intern(NS_SXP, Symbol::create("*in*"), SYS_IN);
    VAR_IN->setDynamic();
    VAR_OUT = Var::intern(NS_SXP, Symbol::create("*out*"), SYS_OUT);
    VAR_OUT->setDynamic();
    VAR_ERR = Var::intern(NS_SXP, Symbol::create("*err*"), SYS_ERR);
    VAR_ERR->setDynamic();
    VAR_CUR_NS = Var::intern(NS_SXP, Symbol::create("*ns*"), NS_SXP);
    VAR_CUR_NS->setDynamic();
    VAR_PRINT_READABLY = Var::intern(NS_SXP,
                                     Symbol::create("*print-readably*"),
                                     rt::T);
    VAR_PRINT_READABLY->setDynamic();
    VAR_FLUSH_ON_NEWLINE = Var::intern(NS_SXP,
                                       Symbol::create("*flush-on-newline*"),
                                       rt::T);
    VAR_FLUSH_ON_NEWLINE->setDynamic();
    VAR_NS = Var::intern(NS_SXP, SYM_NS, rt::F);
    /*
      The compiler somewhat ensures any reference to IN-NS will point to this
      proc.
    */
    Proc* proc = Proc::create("in-ns");
    proc->addMethod(false, 1, vasm::IN_NS_1);
    VAR_IN_NS = Var::intern(NS_SXP, SYM_IN_NS, proc)
        ->withMeta(Hashmap::create({
                    KW_DOC,
                    String::create("Set *ns* to the namespace named by sym,"
                                   " creating it if needed."),
                    KW_PARAMS,
                    String::create("([sym])")
                }));
    //
    Proc::initProcs();
    CFn::initCFns();
    // 
    atexit(shutdown);
    loadFile("./sxpsrc/core.sxp");
    // Keyword::dumpCache();
    // String::dumpCache();
}

Namespace* sxpNS() {
    return NS_SXP;
}

void warning(const std::string& msg) {
    SYS_ERR->println("WARNING: " + msg);
}

Namespace* currentNS(Namespace* ns) {
    if (ns)
        return pNamespace(VAR_CUR_NS->set(ns));
    else
        return pNamespace(VAR_CUR_NS->get());
}

std::string typeName(Obj* obj) {
    if (obj == NIL)
        return "SxNil";
    return obj->typeName();
}

bool isSpecial(Obj* obj) {
    if (Symbol* s = pSymbol(obj))
        return specials.find(s) != specials.end();
    return false;
}

size_t nextID() {
    return uniqueID++;
}

std::string genName(const std::string& prefix, const std::string& suffix) {
    assert((!prefix.empty() || !suffix.empty())
           && "GENSYM MUST HAVE A PREFIX OR SUFFIX");
    std::stringstream ss;
    ss << prefix << nextID() << suffix;
    return ss.str();
}

Symbol* genSym(const std::string& prefix, const std::string& suffix) {
    return Symbol::create(genName(prefix, suffix));
}

void loadFile(const std::string& fileName) {
    VM vm;
    Obj* x;
    FStream* s = FStream::create(fileName, std::ios_base::in);
    DynScope ds(VAR_CUR_NS, currentNS());
    while ((x = reader::readOne(s, false, SENTINEL)) != SENTINEL)
        // std::cout << toString(x) << std::endl;
        vm.run(compiler::compile(x));
}

Obj* get(Obj* coll, Obj* key, Obj* notFound) {
    if (!coll)
        return NIL;
    if (IAssociative* p = pIAssociative(coll))
        return p->valAt(key, notFound);
    if (IIndexed* p = pIIndexed(coll))
        if (INumber* pp = pINumber(key))
            return p->nth(pp->toInt(), notFound);
    if (ISet* p = pISet(coll))
        return p->get(key, notFound);
    return NIL;
}

Obj* currentIN() {
    return VAR_IN->get();
}

Obj* currentOUT() {
    return VAR_OUT->get();
}

Obj* currentERR() {
    return VAR_ERR->get();
}

bool printReadably() {
    return toBool(VAR_PRINT_READABLY->get());
}

bool flushOnNewline() {
    return toBool(VAR_FLUSH_ON_NEWLINE->get());
}


// =========================================================================
// IObj

std::string toString(Obj* obj) {
    if (obj == NIL)
        return "nil";
    return obj->toString();
}

bool toBool(Obj* obj) {
    if (obj == NIL || obj == rt::F)
        return false;
    return true;
}

bool isEqualTo(Obj* o1, Obj* o2) {
    if (o1 == NIL)
        return o2 == NIL;
    return o1->isEqualTo(o2);
}

// =========================================================================
// IMeta

Hashmap* meta(Obj* obj) {
    if (IMeta* p = pIMeta(obj))
        return p->meta();
    throw SxNotImplementedError(typeName(obj) + " does not implemnt IMeta");
}

Obj* withMeta(Obj* obj, Hashmap* m) {
    if (IMeta* p = pIMeta(obj))
        return p->withMeta(m);
    throw SxNotImplementedError(typeName(obj) + " does not implemnt IMeta");
}

// =========================================================================
// ISeq

Obj* first(Obj* obj) {
    if (obj == NIL)
        return NIL;
    else if (ISeq* s = pISeq(obj))
        return s->first();
    ISeq* s = seq(obj);         // may throw
    if (s)
        return s->first();
    return s;
}

ISeq* rest(Obj* obj) {
    if (obj == NIL)
        return List::create();      // (), not nil
    else if (ISeq* s = pISeq(obj))
        return s->rest();
    ISeq* s = seq(obj);         // may throw
    if (s)
        return s->rest();
    return List::create();
}

ISeq* next(Obj* obj) {
    if (obj == NIL)
        return NIL;
    else if (ISeq* s = pISeq(obj))
        return s->next();
    ISeq* s = seq(obj);         // may throw
    if (s)
        return s->next();
    return NIL;
}

ISeq* cons(Obj* coll, Obj* obj) {
    if (coll == NIL)
        return List::create(obj);
    else if (ISeq* s = pISeq(coll))
        return s->cons(obj);
    ISeq* s = seq(coll);        // may throw
    if (s)
        return s->cons(obj);
    return List::create(obj);
}

// =========================================================================
// ISeqable

ISeq* seq(Obj* obj) {
    if (obj == NIL)
        return NIL;
    if (ISeqable* s = pISeqable(obj))
        return s->seq();
    std::stringstream ss;
    ss << obj->typeName() << " does not implement ISeqable";
    throw SxNotImplementedError(ss.str());
}

// =========================================================================
// IIndexed

Obj* nth(Obj* obj, int i) {
    if (IIndexed* p = cpIIndexed(obj))
        return p->nth(i);
    std::stringstream ss;
    ss << ((obj == NIL) ? "SxNil" : obj->typeName())
       << " does not implement IIndexed";
    throw SxNotImplementedError(ss.str());
}

Obj* nth(Obj* obj, int i, Obj* notFound) {
    if (IIndexed* p = cpIIndexed(obj))
        return p->nth(i, notFound);
    std::stringstream ss;
    ss << ((obj == NIL) ? "SxNil" : obj->typeName())
       << " does not implement IIndexed";
    throw SxNotImplementedError(ss.str());
}

// =========================================================================
// ICollection

int count(Obj* obj) {
    if (obj == NIL)
        return 0;
    if (ICollection* c = pICollection(obj))
        return c->count();
    std::stringstream ss;
    ss << obj->typeName() << " does not implement ICollection";
    throw SxNotImplementedError(ss.str());
}

bool isEmpty(Obj* obj) {
    if (obj == NIL)
        return true;
    if (ICollection* c = pICollection(obj))
        return c->isEmpty();
    std::stringstream ss;
    ss << obj->typeName() << " does not implement ICollection";
    throw SxNotImplementedError(ss.str());
}

ICollection* conj(Obj* coll, Obj* obj) {
    if (coll == NIL)
        return List::create(obj);
    if (ICollection* c = pICollection(coll))
        return c->conj(obj);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement ICollection";
    throw SxNotImplementedError(ss.str());
}

// =========================================================================
// IAssociative

IAssociative* assoc(Obj* coll, Obj* key, Obj* val) {
    if (coll == NIL) {
        Hashmap* m = Hashmap::create();
        m->assoc(key, val);
        return m;
    }
    else if (IAssociative* p = pIAssociative(coll))
        return p->assoc(key, val);
    // else if (IIndexed* p pIIndexed(coll))
    //     return p->setNth(key, val);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement IAssociative or IIndexed";
    throw SxNotImplementedError(ss.str());
}

IAssociative* dissoc(Obj* coll, Obj* key) {
    if (coll == NIL)
        return NIL;
    else if (IAssociative* a = pIAssociative(coll))
        return a->dissoc(key);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement IAssociative";
    throw SxNotImplementedError(ss.str());
}

bool hasKey(Obj* coll, Obj* key) {
    if (coll == NIL)
        return false;
    else if (IAssociative* a = pIAssociative(coll))
        return a->hasKey(key);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement IAssociative";
    throw SxNotImplementedError(ss.str());
}

MapEntry* entryAt(Obj* coll, Obj* key) {
    if (coll == NIL)
        return NIL;
    else if (IAssociative* a = pIAssociative(coll))
        return a->entryAt(key);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement IAssociative";
    throw SxNotImplementedError(ss.str());
}

Obj* valAt(Obj* coll, Obj* key, Obj* notFound) {
    if (coll == NIL)
        return notFound;
    else if (IAssociative* a = pIAssociative(coll))
        return a->valAt(key);
    std::stringstream ss;
    ss << coll->typeName() << " does not implement IAssociative";
    throw SxNotImplementedError(ss.str());
}

// =========================================================================
// ICopy

Obj* copy(Obj* obj) {
    if (obj == NIL)
        return NIL;
    return obj->copy();
    /*
      if (ICopy* p = pICopy(obj))
      return p->copy();
      std::stringstream ss;
      ss << (obj ? obj->typeName() : "nil") << " does not implement ICopy";
      throw SxNotImplementedError(ss.str());
    */
}

// =========================================================================

Obj* second(Obj* x) {
    return first(next(x));
}

Obj* third(Obj* x) {
    return first(next(next(x)));
}

ISeq* keys(Hashmap* m) {          // for Var::pushBindings
    ISeq* s = List::create();
    for (auto e : m->impl())
        s = s->cons(e.first);
    return rt::seq(s);
}

bool contains(Obj* coll, Obj* key) {
    if (coll == NIL)
        return false;
    else if (IAssociative* p = pIAssociative(coll))
        return p->hasKey(key);
    else if (ISet* p = pISet(coll))
        return p->contains(key);
    else if (INumber* pn = pINumber(key))
        if (pIIndexed(coll)) {
            long n = pn->toInt();
            return n >= 0 && n < count(coll);
        }
    return false;
}

} // end namespace RT
