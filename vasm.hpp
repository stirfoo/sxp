/*
  vasm.hpp
  S. Edward Dolan
  Sunday, May 14 2023
*/

#ifndef VASM_HPP_INCLUDED
#define VASM_HPP_INCLUDED

namespace vasm {

enum Opcode {
    HALT,
    POP,
    DUP,
    LOAD_NIL, LOAD_TRUE, LOAD_FALSE,
    LOAD_EMPTY_LIST, LOAD_EMPTY_VECTOR, LOAD_EMPTY_HASHMAP, LOAD_EMPTY_HASHSET,
    NEW_VECTOR, NEW_HASHMAP, NEW_LIST, NEW_HASHSET,
    JUMP, JUMP_IF_FALSE,
    LOAD_CONST_0, LOAD_CONST_1, LOAD_CONST_2, LOAD_CONST_3, LOAD_CONST_4,
    LOAD_CONST_B, LOAD_CONST_S,
    LOAD_LOCAL_0, LOAD_LOCAL_1, LOAD_LOCAL_2, LOAD_LOCAL_3, LOAD_LOCAL_4,
    LOAD_LOCAL_B, LOAD_LOCAL_S,
    STORE_LOCAL_0, STORE_LOCAL_1, STORE_LOCAL_2, STORE_LOCAL_3, STORE_LOCAL_4,
    STORE_LOCAL_B, STORE_LOCAL_S,
    LOAD_FREE_0, LOAD_FREE_1, LOAD_FREE_2, LOAD_FREE_3, LOAD_FREE_4,
    LOAD_FREE_B,
    STORE_FREE_0, STORE_FREE_1, STORE_FREE_2, STORE_FREE_3, STORE_FREE_4,
    STORE_FREE_B,
    NEW_CLOSURE,
    DEF,
    VAR_GET,
    CALL_0, CALL_1, CALL_2, CALL_3, CALL_4, CALL_B, CALL_S,
    CALL_PROC,
    CALL_CFN,
    RETURN,
    SET_META,
    APPLY,
    SWAP, SWAP2,
    THROW, RETHROW,
    VAR_SET,                    // 0x68
    JSR, RET
};

enum ProcID {
    // for quasi-quote
    SEQ_1 = 256,
    FIRST_1, REST_1, NEXT_1,
    CONJ_2N,
    LOAD_1,
    CONCAT_0N,
    LIST_0N,
    VECTOR_0N,
    HASHMAP_0N,
    HASHSET_0N,
    //
    TREEMAP_0N,
    TREESET_0N,
    TYPENAME_1,
    EQ_1, EQ_2, EQ_2N,          // (= ...)
    VM_TRACE_0, VM_STACK_0,
    COMPILE_1,
    IDENTICAL_P_2,
    META_1, WITH_META_2,
    KEY_1, VAL_1,
    // Namespace procs
    NS_MAP_1,
    IN_NS_1,
    REFER_1,
    NS_NAME_1,
    FIND_NS_1,
    NS_PUBLICS_1,
    // 
    COUNT_1,
    FN_DUMP_1,
    READ_0, READ_1, READ_3,
    READ_CHAR_1,
    STR_0, STR_1, STR_1N,
    NTH_2, NTH_3,
    GET_2, GET_3,
    ASSOC_3N,
    DISSOC_1N,
    MAKE_LAZY_SEQ_1,
    KEYWORD_1,
    SYMBOL_1, SYMBOL_2,
    NEXT_ID_0,
    ERROR_0, ERROR_1, ERROR_2, ERR_MSG_1,
    CONTAINS_P_2,
    COPY_1,
    NAME_1,
    // regex
    RE_MATCH_2,
    RE_MATCH_3,
    RE_MATCH_4,
    RE_PATTERN_1,
    // io
    PR_0N,
    NEWLINE_0,                  // ignores *print-readably*
    PRINTLN_1,
    FSTREAM_1N,
    SSTREAM_0,
    SSTREAM_1N,
    FCLOSE_1,
    SLURP_1,
    READ_LINE_0,
    // 
    PUSH_BINDINGS_1, POP_BINDINGS_0,
    MACROEXPAND_1_1,
    // interface predicates
    SEQ_P_1,
    SEQABLE_P_1,
    NUMBER_P_1,
    INDEXED_P_1,
    COLLECTION_P_1,
    ASSOCIATIVE_P_1,
    SORTABLE_P_1,
    STREAM_P_1,
    INSTREAM_P_1,
    OUTSTREAM_P_1,
    IOSTREAM_P_1, 
    SET_P_1,
    // type predicates
    ERROR_P_1,
    IO_ERROR_P_1,
    BOOL_P_1,
    CFN_P_1,
    CHARACTER_P_1,
    CLOSURE_P_1,
    FLOAT_P_1,
    FN_P_1,
    FSTREAM_P_1,
    HASHMAP_P_1,
    HASHSET_P_1,
    INTEGER_P_1,
    KEYWORD_P_1,
    LIST_P_1,
    MAP_P_1,
    MAPENTRY_P_1,
    NAMESPACE_P_1,
    PROC_P_1,
    REGEX_P_1,
    SSTREAM_P_1,
    STRING_P_1,
    SYMBOL_P_1,
    TREEMAP_P_1,
    TREESET_P_1,
    VAR_P_1,
    VECTOR_P_1,
    // numbers
    INT_1, FLOAT_1,
    ADD_0, ADD_1, ADD_2, ADD_2N, // (+ ...)
    SUB_1, SUB_2, SUB_2N,        // (- ...)
    MUL_0, MUL_1, MUL_2, MUL_2N, // (* ...)
    DIV_1, DIV_2, DIV_2N,        // (/ ...)
    EQEQ_1, EQEQ_2, EQEQ_2N,     // (== ...)
    LT_1, LT_2, LT_2N,           // (< ...)
};

int disOne(const FnMethod* m, int addr, std::ostream& s=std::cout);
void dis(const FnMethod* m, std::ostream& s=std::cout);


} // end namespace vasm

#endif // VASM_HPP_INCLUDED
