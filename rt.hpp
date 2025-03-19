/*
  rt.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef RT_HPP_INCLUDED
#define RT_HPP_INCLUDED

namespace rt {

extern bool vmTrace;

extern Bool* T;
extern Bool* F;

extern SysInStream* SYS_IN;
extern SysOutStream* SYS_OUT;
extern SysOutStream* SYS_ERR;

extern Obj* UNBOUND;
extern Obj* SENTINEL;
extern INumber* INT_ZERO;
extern INumber* INT_ONE;

extern Symbol* SYM_UNQUOTE;
extern Symbol* SYM_UNQUOTE_SPLICING;
extern Symbol* SYM_LIST;
extern Symbol* SYM_CONCAT;
extern Symbol* SYM_SEQ;
extern Symbol* SYM_VECTOR;
extern Symbol* SYM_HASHMAP;
extern Symbol* SYM_HASHSET;
extern Symbol* SYM_NS;
extern Symbol* SYM_IN_NS;

// special symbols
extern Symbol* SYM_QUOTE;
extern Symbol* SYM_DO;
extern Symbol* SYM_IF;
extern Symbol* SYM_DEF;
extern Symbol* SYM_SET_BANG;      // set!
extern Symbol* SYM_LET;
extern Symbol* SYM_FN;
extern Symbol* SYM_LOOP;
extern Symbol* SYM_RECUR;
extern Symbol* SYM_APPLY;
extern Symbol* SYM_LETFN;
extern Symbol* SYM_VAR;
extern Symbol* SYM_TRY;
extern Symbol* SYM_CATCH;
extern Symbol* SYM_FINALLY;
extern Symbol* SYM_THROW;
extern Symbol* SYM_AMP;        // & so (fn [& x] ...) not (fn [sxp/& x] ...)

extern Var* VAR_NS;
extern Var* VAR_IN_NS;

extern Var* VAR_PRINT_READABLY;

extern Keyword* KW_MACRO;
extern Keyword* KW_PRIVATE;
extern Keyword* KW_TAG;
extern Keyword* KW_DOC;
extern Keyword* KW_PARAMS;
extern Keyword* KW_ONCE;
extern Keyword* KW_DYNAMIC;
// stream open mode flags
extern Keyword* KW_APP;
extern Keyword* KW_BINARY;
extern Keyword* KW_IN;
extern Keyword* KW_OUT;
extern Keyword* KW_TRUNC;
extern Keyword* KW_ATE;

extern Hashmap* DEFAULT_IMPORTS;

static std::set<Symbol*, ObjLessFntr> specials;

void pushVM(VM*);
void popVM();
VM* currentVM();

void init();
void gc();

Namespace* sxpNS();
void warning(const std::string& msg);
Namespace* currentNS(Namespace* ns = nullptr);
bool printReadably();
std::string typeName(Obj*);
bool isSpecial(Obj*);
std::string genName(const std::string& prefix, const std::string& suffix);
Symbol* genSym(const std::string& prefix, const std::string& suffix);
size_t nextID();
void loadFile(const std::string& fileName);

Obj* currentIN();
Obj* currentOUT();
Obj* currentERR();

bool printReadably();
bool flushOnNewline();

// IObj
std::string toString(Obj*);
bool toBool(Obj*);
size_t getHash(Obj*);
bool isEqualTo(Obj*, Obj*);

// IMeta
Hashmap* meta(Obj*);
Obj* withMeta(Obj*, Hashmap*);
    
// ISeq
Obj* first(Obj*);
ISeq* rest(Obj*);
ISeq* next(Obj*);
ISeq* cons(Obj*, Obj*);

// ISeqable
ISeq* seq(Obj*);

// IIndexed
Obj* nth(Obj*, int);
Obj* nth(Obj*, int, Obj*);

// ICollection
int count(Obj*);
bool isEmpty(Obj*);
ICollection* conj(Obj*, Obj*);

// IAssociative
IAssociative* assoc(Obj*, Obj*, Obj*);
IAssociative* dissoc(Obj*, Obj*);
bool hasKey(Obj*, Obj*);
MapEntry* entryAt(Obj*, Obj*);
Obj* valAt(Obj*, Obj*, Obj* notFound=NIL);

// ICopy
Obj* copy(Obj*);

Obj* get(Obj*, Obj*, Obj* notFound=NIL);

//
Obj* second(Obj*);
Obj* third(Obj*);
ISeq* keys(Hashmap*);
bool contains(Obj* coll, Obj* key);

} // end namespace rt

#endif // RT_HPP_INCLUDED
