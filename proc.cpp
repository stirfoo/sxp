/*
  proc.cpp
  S. Edward Dolan
  Wednesday, May 24 2023
*/

#include "sxp.hpp"

Proc* Proc::create(const std::string& name) {
    return new Proc(name);
}

Proc::Proc(const std::string& name)
    : Fn(name) {
    _typeName = "SxProc";
}

std::string Proc::toString() {
    std::stringstream ss;
    ss << "#<" << typeName() << ' ' << _name << ' ' << this << '>';
    return ss.str();
}

FnMethod* Proc::addMethod(bool isRest, int reqArgs, uint16_t id) {
    FnMethod* m = Fn::addMethod(isRest, reqArgs);
    m->nLocals(1 + reqArgs + (isRest ? 1 : 0));
    m->appendByte(vasm::CALL_PROC);
    m->appendByte(static_cast<uint8_t>(id));
    m->appendByte(static_cast<uint8_t>(id >> 8));
    m->appendByte(vasm::RETURN);
    return m;
}

// =========================================================================

#define MAKPRC(name, params, doc)                                       \
    proc = Proc::create(name);                                          \
    Var::intern(rt::sxpNS(), Symbol::create(name), proc)               \
    ->withMeta(Hashmap::create({                                        \
                rt::KW_DOC,                                             \
                String::create(doc),                                    \
                rt::KW_PARAMS,                                          \
                String::create("(" + std::string(params) + ")")         \
            }))

static void initNumberProcs() {
    Proc* proc = nullptr;
    MAKPRC("int", "[x]", "Coerce x to an integer.");
    proc->addMethod(false, 1, vasm::INT_1);
    MAKPRC("float", "[x]", "Coerce x to a floating point number.");
    proc->addMethod(false, 1, vasm::FLOAT_1);
    MAKPRC("+", "[] [x] [x y] [x y & zs]", "Return the sum of the numbers.");
    proc->addMethod(false, 0, vasm::ADD_0);
    proc->addMethod(false, 1, vasm::ADD_1);
    proc->addMethod(false, 2, vasm::ADD_2);
    proc->addMethod(true, 2, vasm::ADD_2N);
    MAKPRC("-", "[x] [x y] [x y & zs]", "Return the difference of the"
           " numbers");
    proc->addMethod(false, 1, vasm::SUB_1);
    proc->addMethod(false, 2, vasm::SUB_2);
    proc->addMethod(true, 2, vasm::SUB_2N);
    MAKPRC("*", "[] [x] [x y] [x y & zs]", "Return the product of the"
           " numbers");
    proc->addMethod(false, 0, vasm::MUL_0);
    proc->addMethod(false, 1, vasm::MUL_1);
    proc->addMethod(false, 2, vasm::MUL_2);
    proc->addMethod(true, 2, vasm::MUL_2N);
    MAKPRC("/", "[x] [x y] [x y & zs]", "Return the quotient of the numbers.");
    proc->addMethod(false, 1, vasm::DIV_1);
    proc->addMethod(false, 2, vasm::DIV_2);
    proc->addMethod(true, 2, vasm::DIV_2N);
    MAKPRC("==", "[x] [x y] [x y & zs]", "Return true if all numbers are"
           " equal");
    proc->addMethod(false, 1, vasm::EQEQ_1);
    proc->addMethod(false, 2, vasm::EQEQ_2);
    proc->addMethod(true, 2, vasm::EQEQ_2N);
    MAKPRC("<", "[x] [x y] [x y & zs]", "Return true if each number,"
           " left-to-right is smaller than the next.");
    proc->addMethod(false, 1, vasm::LT_1);
    proc->addMethod(false, 2, vasm::LT_2);
    proc->addMethod(true, 2, vasm::LT_2N);
}

static void initPredicateProcs() {
    Proc* proc = nullptr;
    // Interface type predicates
    MAKPRC("seq?", "[x]", "Return true if x implements ISeq");
    proc->addMethod(false, 1, vasm::SEQ_P_1);
    MAKPRC("seqable?", "[x]", "Return true if x implements ISeqable");
    proc->addMethod(false, 1, vasm::SEQABLE_P_1);
    MAKPRC("number?", "[x]", "Return true if x implements INumber");
    proc->addMethod(false, 1, vasm::NUMBER_P_1);
    MAKPRC("indexed?", "[x]", "Return true if x implements IIndexed");
    proc->addMethod(false, 1, vasm::INDEXED_P_1);
    MAKPRC("collection?", "[x]", "Return true if x implements ICollection");
    proc->addMethod(false, 1, vasm::COLLECTION_P_1);
    MAKPRC("associative?", "[x]", "Return true if x implements IAssociative");
    proc->addMethod(false, 1, vasm::ASSOCIATIVE_P_1);
    MAKPRC("sortable?", "[x]", "Return true if x implements ISortable");
    proc->addMethod(false, 1, vasm::SORTABLE_P_1);
    MAKPRC("stream?", "[x]", "Return true if x implements IStream");
    proc->addMethod(false, 1, vasm::STREAM_P_1);
    MAKPRC("instream?", "[x]", "Return true if x implements IInStream");
    proc->addMethod(false, 1, vasm::INSTREAM_P_1);
    MAKPRC("outstream?", "[x]", "Return true if x implements IOutStream");
    proc->addMethod(false, 1, vasm::OUTSTREAM_P_1);
    MAKPRC("set?", "[x]", "Return true if x implements ISet");
    proc->addMethod(false, 1, vasm::SET_P_1);
    // Type predicates
    MAKPRC("error?", "[x]", "Return true if x is an SxError.");
    proc->addMethod(false, 1, vasm::ERROR_P_1);
    MAKPRC("io-error?", "[x]", "Return true if x is an SxIOError.");
    proc->addMethod(false, 1, vasm::IO_ERROR_P_1);
    MAKPRC("bool?", "[x]", "Return true if x is an SxBool.");
    proc->addMethod(false, 1, vasm::BOOL_P_1);
    MAKPRC("cfn?", "[x]", "Return true if x is an SxCFn.");
    proc->addMethod(false, 1, vasm::CFN_P_1);
    MAKPRC("character?", "[x]", "Return true if x is an SxCharacter.");
    proc->addMethod(false, 1, vasm::CHARACTER_P_1);
    MAKPRC("closure?", "[x]", "Return true if x is an SxClosure.");
    proc->addMethod(false, 1, vasm::CLOSURE_P_1);
    MAKPRC("float?", "[x]", "Return true if x is an SxFloat.");
    proc->addMethod(false, 1, vasm::FLOAT_P_1);
    MAKPRC("fn?", "[x]", "Return true if x is an SxFn.");
    proc->addMethod(false, 1, vasm::FN_P_1);
    MAKPRC("fstream?", "[x]", "Return true if x is an SxFstream.");
    proc->addMethod(false, 1, vasm::FSTREAM_P_1);
    MAKPRC("hashmap?", "[x]", "Return true if x is an SxHashmap.");
    proc->addMethod(false, 1, vasm::HASHMAP_P_1);
    MAKPRC("hashset?", "[x]", "Return true if x is an SxHashset.");
    proc->addMethod(false, 1, vasm::HASHSET_P_1);
    MAKPRC("integer?", "[x]", "Return true if x is an SxInteger.");
    proc->addMethod(false, 1, vasm::INTEGER_P_1);
    MAKPRC("keyword?", "[x]", "Return true if x is an SxKeyword.");
    proc->addMethod(false, 1, vasm::KEYWORD_P_1);
    MAKPRC("list?", "[x]", "Return true if x is an SxList.");
    proc->addMethod(false, 1, vasm::LIST_P_1);
    MAKPRC("map?", "[x]", "Return true if x is an SxHashmap or SxTreemap.");
    proc->addMethod(false, 1, vasm::MAP_P_1);
    MAKPRC("mapentry?", "[x]", "Return true if x is an SxMapEntry.");
    proc->addMethod(false, 1, vasm::MAPENTRY_P_1);
    MAKPRC("namespace?", "[x]", "Return true if x is an SxNamespace.");
    proc->addMethod(false, 1, vasm::NAMESPACE_P_1);
    MAKPRC("proc?", "[x]", "Return true if x is an SxProc.");
    proc->addMethod(false, 1, vasm::PROC_P_1);
    MAKPRC("regex?", "[x]", "Return true if x is an SxRegex.");
    proc->addMethod(false, 1, vasm::REGEX_P_1);
    MAKPRC("sstream?", "[x]", "Return true if x is an SxSstream.");
    proc->addMethod(false, 1, vasm::SSTREAM_P_1);
    MAKPRC("string?", "[x]", "Return true if x is an SxString.");
    proc->addMethod(false, 1, vasm::STRING_P_1);
    MAKPRC("symbol?", "[x]", "Return true if x is an SxSymbol.");
    proc->addMethod(false, 1, vasm::SYMBOL_P_1);
    MAKPRC("treemap?", "[x]", "Return true if x is an SxTreemap.");
    proc->addMethod(false, 1, vasm::TREEMAP_P_1);
    MAKPRC("treeset?", "[x]", "Return true if x is an SxTreeset.");
    proc->addMethod(false, 1, vasm::TREESET_P_1);
    MAKPRC("var?", "[x]", "Return true if x is an SxVar.");
    proc->addMethod(false, 1, vasm::VAR_P_1);
    MAKPRC("vector?", "[x]", "Return true if x is an SxVector or SxMapEntry.");
    proc->addMethod(false, 1, vasm::VECTOR_P_1);
}

static void initIOProcs() {
    Proc* proc = nullptr;
    MAKPRC("load", "[file-name]", "Read and evaluate the sxp expression found"
           " in the file.");
    proc->addMethod(false, 1, vasm::LOAD_1);
    MAKPRC("fstream", "[file-name & modes]",
           "Return a file stream open on the given file name. MODE will"
           " default to :in if not supplied");
    proc->addMethod(true, 1, vasm::FSTREAM_1N);
    MAKPRC("sstream", "[] [init-str & modes]",
           "Return a string stream given the optional initialization string"
           " and open modes. mode will default to :in :out if not supplied");
    proc->addMethod(false, 0, vasm::SSTREAM_0);
    proc->addMethod(true, 1, vasm::SSTREAM_1N);
    MAKPRC("read", "[] [stream] [stream eof-is-error? eof-value]",
           "Read one object from the given stream or the current value of"
           " *IN* if stream is not provided.");
    proc->addMethod(false, 0, vasm::READ_0);
    proc->addMethod(false, 1, vasm::READ_1);
    proc->addMethod(false, 3, vasm::READ_3);
    MAKPRC("pr", "[& xs]", "Print xs to the current value of *out*"
           " interspersed with a single space. Return nil.");
    proc->addMethod(true, 0, vasm::PR_0N);
    MAKPRC("newline", "[]", "Write a single newline to the current value"
           " of *out*. Reutn nil.");
    proc->addMethod(false, 0, vasm::NEWLINE_0);
    MAKPRC("read-char", "[stream]", "Read and return the single characer"
           " read from the stream");
    proc->addMethod(false, 1, vasm::READ_CHAR_1);
    MAKPRC("fclose", "[stream]", "Close the stream. Return nil.");
    proc->addMethod(false, 1, vasm::FCLOSE_1);
    MAKPRC("slurp", "[file-name]", "Open the file, read its contents into a"
           " string, close the file, return the string.");
    proc->addMethod(false, 1, vasm::SLURP_1);
    MAKPRC("read-line", "[]", "Read and return the next line from *in*."
           " Return nil on end of input. The newline is omitted.");
    proc->addMethod(false, 0, vasm::READ_LINE_0);
}

static void initNSProcs() {
    Proc* proc = nullptr;
    MAKPRC("ns-name", "[ns]", "Return the name of the supplied namespace");
    proc->addMethod(false, 1, vasm::NS_NAME_1);
    MAKPRC("ns-map", "[ns]", "Return the mappings for the given namespace.");
    proc->addMethod(false, 1, vasm::NS_MAP_1);
    MAKPRC("refer", "[ns-sym]", "TODO...");
    proc->addMethod(false, 1, vasm::REFER_1);
    MAKPRC("find-ns", "[sym]", "Return the namespace named by sym or nil if"
           " not found.");
    proc->addMethod(false, 1, vasm::FIND_NS_1);
    MAKPRC("ns-publics", "[ns-name]", "Return a (possibly empty) map of all"
           " public interned mapping in the namespace.");
    proc->addMethod(false, 1, vasm::NS_PUBLICS_1);
}

static void initVarProcs() {
    Proc* proc = nullptr;
    MAKPRC("push-bindings", "[map]",
           "Dynamically bind the keys (vars) to the vals in the map. This"
           " must be accompanied with a matching POP-BINDINGS. Return nil.");
    proc->addMethod(false, 1, vasm::PUSH_BINDINGS_1);
    MAKPRC("pop-bindings", "[]",
           "Restore the dynamic bindings created by a previous PUSH-BINDINGS."
           " Return nil.");
    proc->addMethod(false, 0, vasm::POP_BINDINGS_0);
}

static void initVMProcs() {
    Proc* proc = nullptr;
    MAKPRC("vm-trace", "[]", "Toggle printed tracing to stdout of the VM"
           " execution.");
    proc->addMethod(false, 0, vasm::VM_TRACE_0);
    MAKPRC("vm-stack", "[]", "Print the contents of the stack to the current"
           " value of *out*");
    proc->addMethod(false, 0, vasm::VM_STACK_0);
}

static void initErrorProcs() {
    Proc* proc = nullptr;
    MAKPRC("error", "[] [message] [type message]", "Return a new instance of"
           " an SxError, or of the supplied type, with the optional message");
    proc->addMethod(false, 0, vasm::ERROR_0);
    proc->addMethod(false, 1, vasm::ERROR_1);
    proc->addMethod(false, 2, vasm::ERROR_2);
    MAKPRC("err-msg", "[error]", "Return the message string of the given"
           " SxError.");
    proc->addMethod(false, 1, vasm::ERR_MSG_1);
}

void initRegexProcs() {
    Proc* proc = nullptr;
    MAKPRC("re-match", "[re str] [re str start] [re str start len]", "");
    proc->addMethod(false, 2, vasm::RE_MATCH_2);
    proc->addMethod(false, 3, vasm::RE_MATCH_3);
    proc->addMethod(false, 4, vasm::RE_MATCH_4);
    MAKPRC("re-pattern", "[re]", "Return the string pattern the regex was"
           " compiled with.");
    proc->addMethod(false, 1, vasm::RE_PATTERN_1);
}

void initCreators() {
    Proc* proc = nullptr;
    MAKPRC("keyword", "[name]", "Intern and return a keyword given the name,"
           " which may be a string or quoted symbol. The colon will be added"
           " by this function."
           " prefix the result with a colon.");
    proc->addMethod(false, 1, vasm::KEYWORD_1);
    MAKPRC("list", "[& xs]", "Return a new list of of the xs.");
    proc->addMethod(true, 0, vasm::LIST_0N);
    MAKPRC("vector", "[& xs]", "Return a new vector of the xs.");
    proc->addMethod(true, 0, vasm::VECTOR_0N);
    MAKPRC("hashmap", "[& keys-vals]", "Return a new hashmap of the given"
           " keys and values.");
    proc->addMethod(true, 0, vasm::HASHMAP_0N);
    MAKPRC("hashset", "[& xs]", "Return a new hashset of the given elements.");
    proc->addMethod(true, 0, vasm::HASHSET_0N);
    MAKPRC("treemap", "[& keys-vals]", "Return a new treemap (keys ordered)"
           " of the given keys and values.");
    proc->addMethod(true, 0, vasm::TREEMAP_0N);
    MAKPRC("treeset", "[& xs]", "Return a new treeset (ordered) of the given"
           " elements.");
    proc->addMethod(true, 0, vasm::TREESET_0N);
}

void Proc::initProcs() {
    initNumberProcs();
    initPredicateProcs();
    initIOProcs();
    initNSProcs();
    initVarProcs();
    initVMProcs();
    initErrorProcs();
    initCreators();
    initRegexProcs();
    Proc* proc = nullptr;
    MAKPRC("seq", "[x]", "Return a sequence (a list) on x, or nil.");
    proc->addMethod(false, 1, vasm::SEQ_1);
    MAKPRC("first", "[x]", "Return the first element of the seq-able x.");
    proc->addMethod(false, 1, vasm::FIRST_1);
    MAKPRC("rest", "[x]", "Return all but the first element of the seq-able"
           " x, or () if empty.");
    proc->addMethod(false, 1, vasm::REST_1);
    MAKPRC("next", "[x]", "Return all but the first element of the seq-able"
           " x, or nil if empty.");
    proc->addMethod(false, 1, vasm::NEXT_1);
    MAKPRC("conj", "[coll x & xs]", "Add items to the collection, in-place.");
    proc->addMethod(true, 2, vasm::CONJ_2N);
    MAKPRC("concat", "[& xs]", "Return a seq of the concatenated values in"
           " each seq-able x.");
    proc->addMethod(true, 0, vasm::CONCAT_0N);
    MAKPRC("typename", "[x]", "Return the string sxp typename of x.");
    proc->addMethod(false, 1, vasm::TYPENAME_1);
    MAKPRC("=", "[x] [x y] [x y & xs]", "Return true if all xs are of equal"
           " value. See: == for numerical equality.");
    proc->addMethod(false, 1, vasm::EQ_1);
    proc->addMethod(false, 2, vasm::EQ_2);
    proc->addMethod(true, 2, vasm::EQ_2N);
    MAKPRC("compile", "[form]", "Compile the unquoted form, return a function"
           " that can be evaluated: ((compile 42)) => 42");
    proc->addMethod(false, 1, vasm::COMPILE_1);
    MAKPRC("identical?", "[x y]", "Return true if x and y are the same"
           " objects in memory.");
    proc->addMethod(false, 2, vasm::IDENTICAL_P_2);
    MAKPRC("str", "[] [x] [x & xs]", "Return a string of the values' string"
           " values. nil return the empty string.");
    proc->addMethod(false, 0, vasm::STR_0);
    proc->addMethod(false, 1, vasm::STR_1);
    proc->addMethod(true, 1, vasm::STR_1N);
    MAKPRC("meta", "[obj]", "Return the meta data of the object.");
    proc->addMethod(false, 1, vasm::META_1);
    MAKPRC("with-meta", "[obj map]",
           "Return obj with the meta data attached.");
    proc->addMethod(false, 2, vasm::WITH_META_2);
    MAKPRC("key", "[map-entry]", "Return the map-entries key.");
    proc->addMethod(false, 1, vasm::KEY_1);
    MAKPRC("val", "[map-entry]", "Return the map-entries val.");
    proc->addMethod(false, 1, vasm::VAL_1);
    MAKPRC("count", "[x]", "Return the number of elements in x.");
    proc->addMethod(false, 1, vasm::COUNT_1);
    MAKPRC("fn-dump", "[fn]", "Print the internal structure of the function.");
    proc->addMethod(false, 1, vasm::FN_DUMP_1);
    MAKPRC("nth", "[coll i] [coll i not-found]",
           "Return the nth elemnt in the collection. If i is out of bounds"
           " and not-found is supplied, retur it, else throw.");
    proc->addMethod(false, 2, vasm::NTH_2);
    proc->addMethod(false, 3, vasm::NTH_3);
    MAKPRC("get", "[map key] [map key not-found]",
           "Return the value at the given key or nil if not-found is not"
           " supplied.");
    proc->addMethod(false, 2, vasm::GET_2);
    proc->addMethod(false, 3, vasm::GET_3);
    MAKPRC("assoc", "[map key val & keysvals]", "Adds the key/val pair to"
           " the map, in place. If map is nil, a new map is created and"
           " returned.");
    proc->addMethod(true, 3, vasm::ASSOC_3N);
    MAKPRC("dissoc", "[map & keys]", "Returns map with all entries at keys"
           " removed.");
    proc->addMethod(true, 1, vasm::DISSOC_1N);
    MAKPRC("make-lazy-seq", "[fn]", "???");
    proc->addMethod(false, 1, vasm::MAKE_LAZY_SEQ_1);
    MAKPRC("symbol", "[name] [ns name]", "Create a new symbol");
    proc->addMethod(false, 1, vasm::SYMBOL_1);
    proc->addMethod(false, 2, vasm::SYMBOL_2);
    MAKPRC("next-id", "[]", "Return the next sequential runtime id.");
    proc->addMethod(false, 0, vasm::NEXT_ID_0);
    MAKPRC("macroexpand-1", "[form]", "Expand form if a macro, else"
           " return x.");
    proc->addMethod(false, 1, vasm::MACROEXPAND_1_1);
    MAKPRC("contains?", "[coll key]",
           "Return true if coll contains key. If key is a number and coll is"
           " indexable, return true if key is a valid index into coll.");
    proc->addMethod(false, 2, vasm::CONTAINS_P_2);
    MAKPRC("copy", "[obj]",
           "Return a shallow copy of obj. If the obj is immutable like nil,"
           " a string, number, etc., return the same instance: (identical?"
           " nil (copy nil)) => true");
    proc->addMethod(false, 1, vasm::COPY_1);
    MAKPRC("name", "[obj]", "Return the name of a symbol, keyword, or string");
    proc->addMethod(false, 1, vasm::NAME_1);
}

#undef MAKPRC
