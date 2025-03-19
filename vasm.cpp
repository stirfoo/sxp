/*
  vasm.cpp
  S. Edward Dolan
  Thursday, May 18 2023
*/

#include "sxp.hpp"

namespace vasm {

enum OperandType {
    NONE, U8, U16
};

struct OpcodeInfo {
    const char* name;
    Opcode opcode;
    int operandCount;
    OperandType type;
};

static std::map<int, OpcodeInfo> opcodeMap = {
    {HALT, {"HALT", HALT, 0, NONE}},
    {POP, {"POP", POP, 0, NONE}},
    {DUP, {"DUP", DUP, 0, NONE}},
    {LOAD_NIL, {"LOAD_NIL", LOAD_NIL, 0, NONE}},
    {LOAD_TRUE, {"LOAD_TRUE", LOAD_TRUE, 0, NONE}},
    {LOAD_FALSE, {"LOAD_FALSE", LOAD_FALSE, 0, NONE}},
    {LOAD_EMPTY_LIST, {"LOAD_EMPTY_LIST", LOAD_EMPTY_LIST, 0, NONE}},
    {LOAD_EMPTY_VECTOR, {"LOAD_EMPTY_VECTOR", LOAD_EMPTY_VECTOR, 0, NONE}},
    {LOAD_EMPTY_HASHMAP, {"LOAD_EMPTY_HASHMAP", LOAD_EMPTY_HASHMAP, 0, NONE}},
    {LOAD_EMPTY_HASHSET, {"LOAD_EMPTY_HASHSET", LOAD_EMPTY_HASHSET, 0, NONE}},
    {NEW_VECTOR, {"NEW_VECTOR", NEW_VECTOR, 0, NONE}},
    {NEW_HASHMAP, {"NEW_HASHMAP", NEW_HASHMAP, 0, NONE}},
    {NEW_LIST, {"NEW_LIST", NEW_LIST, 0, NONE}},
    {NEW_HASHSET, {"NEW_HASHSET", NEW_HASHSET, 0, NONE}},
    {JUMP, {"JUMP", JUMP, 1, U16}},
    {JUMP_IF_FALSE, {"JUMP_IF_FALSE", JUMP_IF_FALSE, 1, U16}},
    {LOAD_CONST_0, {"LOAD_CONST_0", LOAD_CONST_0, 0, NONE}},
    {LOAD_CONST_1, {"LOAD_CONST_1", LOAD_CONST_1, 0, NONE}},
    {LOAD_CONST_2, {"LOAD_CONST_2", LOAD_CONST_2, 0, NONE}},
    {LOAD_CONST_3, {"LOAD_CONST_3", LOAD_CONST_3, 0, NONE}},
    {LOAD_CONST_4, {"LOAD_CONST_4", LOAD_CONST_4, 0, NONE}},
    {LOAD_CONST_B, {"LOAD_CONST_B", LOAD_CONST_B, 1, U8}},
    {LOAD_CONST_S, {"LOAD_CONST_S", LOAD_CONST_S, 1, U16}},
    {LOAD_LOCAL_0, {"LOAD_LOCAL_0", LOAD_LOCAL_0, 0, NONE}},
    {LOAD_LOCAL_1, {"LOAD_LOCAL_1", LOAD_LOCAL_1, 0, NONE}},
    {LOAD_LOCAL_2, {"LOAD_LOCAL_2", LOAD_LOCAL_2, 0, NONE}},
    {LOAD_LOCAL_3, {"LOAD_LOCAL_3", LOAD_LOCAL_3, 0, NONE}},
    {LOAD_LOCAL_4, {"LOAD_LOCAL_4", LOAD_LOCAL_4, 0, NONE}},
    {LOAD_LOCAL_B, {"LOAD_LOCAL_B", LOAD_LOCAL_B, 1, U8}},
    {LOAD_LOCAL_S, {"LOAD_LOCAL_S", LOAD_LOCAL_S, 1, U16}},
    {STORE_LOCAL_0, {"STORE_LOCAL_0", STORE_LOCAL_0, 0, NONE}},
    {STORE_LOCAL_1, {"STORE_LOCAL_1", STORE_LOCAL_1, 0, NONE}},
    {STORE_LOCAL_2, {"STORE_LOCAL_2", STORE_LOCAL_2, 0, NONE}},
    {STORE_LOCAL_3, {"STORE_LOCAL_3", STORE_LOCAL_3, 0, NONE}},
    {STORE_LOCAL_4, {"STORE_LOCAL_4", STORE_LOCAL_4, 0, NONE}},
    {STORE_LOCAL_B, {"STORE_LOCAL_B", STORE_LOCAL_B, 1, U8}},
    {STORE_LOCAL_S, {"STORE_LOCAL_S", STORE_LOCAL_S, 1, U16}},
    {LOAD_FREE_0, {"LOAD_FREE_0", LOAD_FREE_0, 0, NONE}},
    {LOAD_FREE_1, {"LOAD_FREE_1", LOAD_FREE_1, 0, NONE}},
    {LOAD_FREE_2, {"LOAD_FREE_2", LOAD_FREE_2, 0, NONE}},
    {LOAD_FREE_3, {"LOAD_FREE_3", LOAD_FREE_3, 0, NONE}},
    {LOAD_FREE_4, {"LOAD_FREE_4", LOAD_FREE_4, 0, NONE}},
    {LOAD_FREE_B, {"LOAD_FREE_B", LOAD_FREE_B, 1, U8}},
    {STORE_FREE_0, {"STORE_FREE_0", STORE_FREE_0, 0, NONE}},
    {STORE_FREE_1, {"STORE_FREE_1", STORE_FREE_1, 0, NONE}},
    {STORE_FREE_2, {"STORE_FREE_2", STORE_FREE_2, 0, NONE}},
    {STORE_FREE_3, {"STORE_FREE_3", STORE_FREE_3, 0, NONE}},
    {STORE_FREE_4, {"STORE_FREE_4", STORE_FREE_4, 0, NONE}},
    {STORE_FREE_B, {"STORE_FREE_B", STORE_FREE_B, 1, U8}},
    {NEW_CLOSURE, {"NEW_CLOSURE", NEW_CLOSURE, 0, NONE}},
    {DEF, {"DEF", DEF, 0, NONE}},
    {VAR_GET, {"VAR_GET", VAR_GET, 0, NONE}},
    {CALL_0, {"CALL_0", CALL_0, 0, NONE}},
    {CALL_1, {"CALL_1", CALL_1, 0, NONE}},
    {CALL_2, {"CALL_2", CALL_2, 0, NONE}},
    {CALL_3, {"CALL_3", CALL_3, 0, NONE}},
    {CALL_4, {"CALL_4", CALL_4, 0, NONE}},
    {CALL_B, {"CALL_B", CALL_B, 1, U8}},
    {CALL_S, {"CALL_S", CALL_S, 1, U16}},
    {CALL_PROC, {"CALL_PROC", CALL_PROC, 1, U16}},
    {CALL_CFN, {"CALL_CFN", CALL_CFN, 0, NONE}},
    {RETURN, {"RETURN", RETURN, 0, NONE}},
    {SET_META, {"SET_META", SET_META, 0, NONE}},
    {APPLY, {"APPLY", APPLY, 0, NONE}},
    {SWAP, {"SWAP", SWAP, 0, NONE}},
    {SWAP2, {"SWAP2", SWAP2, 0, NONE}},
    {THROW, {"THROW", THROW, 0, NONE}},
    {RETHROW, {"RETHROW", RETHROW, 0, NONE}},
    {VAR_SET, {"VAR_SET", VAR_SET, 0, NONE}}, // 68 instructions
    {JSR, {"JSR", JSR, 1, U16}},
    {RET, {"RET", RET, 0, NONE}},
};

/*
  NOTE: Should be kept in sync with the ProcID enums in vasm.hpp. I say
        `should' because the disOne() code won't crash if the ProcID hasn't
        been added to this map. The name will simply be omitted from the
        trace.
*/
static std::map<int, std::string> procNames = {
    {SEQ_1, "SEQ_1"},
    {FIRST_1, "FIRST_1"},
    {REST_1, "REST_1"},
    {NEXT_1, "NEXT_1"},
    {CONJ_2N, "CONJ_2N"},
    {LOAD_1, "LOAD_1"},
    {CONCAT_0N, "CONCAT_0N"},
    {LIST_0N, "LIST_0N"},
    {VECTOR_0N, "VECTOR_0N"},
    {HASHMAP_0N, "HASHMAP_0N"},
    {HASHSET_0N, "HASHSET_0N"},
    {TREEMAP_0N, "TREEMAP_0N"},
    {TREESET_0N, "TREESET_0N"},
    {TYPENAME_1, "TYPENAME_1"},
    {EQ_1, "EQ_1"},
    {EQ_2, "EQ_2"},
    {EQ_2N, "EQ_2N"},
    {VM_TRACE_0, "VM_TRACE_0"},
    {VM_STACK_0, "VM_STACK_0"},
    {COMPILE_1, "COMPILE_1"},
    {IDENTICAL_P_2, "IDENTICAL_P_2"},
    {META_1, "META_1"},
    {WITH_META_2, "WITH_META_2"},
    {KEY_1, "KEY_1"},
    {VAL_1, "VAL_1"},
    {NS_MAP_1, "NS_MAP_1"},
    {IN_NS_1, "IN_NS_1"},
    {REFER_1, "REFER_1"},
    {NS_NAME_1, "NS_NAME_1"},
    {FIND_NS_1, "FIND_NS_1"},
    {NS_PUBLICS_1, "NS_PUBLICS_1"},
    {COUNT_1, "COUNT_1"},
    {FN_DUMP_1, "FN_DUMP_1"},
    {READ_0, "READ_0"},
    {READ_1, "READ_1"},
    {READ_3, "READ_3"},
    {READ_CHAR_1, "READ_CHAR_1"},
    {STR_0, "STR_0"},
    {STR_1, "STR_1"},
    {STR_1N, "STR_1N"},
    {NTH_2, "NTH_2"},
    {NTH_3, "NTH_3"},
    {GET_2, "GET_2"},
    {ASSOC_3N, "ASSOC_3N"},
    {DISSOC_1N, "DISSOC_1N"},
    {MAKE_LAZY_SEQ_1, "MAKE_LAZY_SEQ_1"},
    {KEYWORD_1, "KEYWORD_1"},
    {SYMBOL_1, "SYMBOL_1"},
    {SYMBOL_2, "SYMBOL_2"},
    {NEXT_ID_0, "NEXT_ID_0"},
    {ERROR_0, "ERROR_0"},
    {ERROR_1, "ERROR_1"},
    {ERROR_2, "ERROR_2"},
    {ERR_MSG_1, "ERR_MSG_1"},
    {CONTAINS_P_2, "CONTAINS_P_2"},
    {COPY_1, "COPY_1"},
    {NAME_1, "NAME_1"},
    {RE_MATCH_2, "RE_MATCH_2"},
    {RE_MATCH_3, "RE_MATCH_3"},
    {RE_MATCH_4, "RE_MATCH_4"},
    {RE_PATTERN_1, "RE_PATTERN_1"},
    {PR_0N, "PR_0N"},
    {NEWLINE_0, "NEWLINE_0"},
    {PRINTLN_1, "PRINTLN_1"},
    {FSTREAM_1N, "FSTREAM_1N"},
    {SSTREAM_0, "SSTREAM_0"},
    {SSTREAM_1N, "SSTREAM_1N"},
    {FCLOSE_1, "FCLOSE_1"},
    {SLURP_1, "SLURP_1"},
    {READ_LINE_0, "READ_LINE_0"},
    {PUSH_BINDINGS_1, "PUSH_BINDINGS_1"},
    {POP_BINDINGS_0, "POP_BINDINGS_0"},
    {MACROEXPAND_1_1, "MACROEXPAND_1_1"},
    {SEQ_P_1, "SEQ_P_1"},
    {SEQABLE_P_1, "SEQABLE_P_1"},
    {NUMBER_P_1, "NUMBER_P_1"},
    {INDEXED_P_1, "INDEXED_P_1"},
    {COLLECTION_P_1, "COLLECTION_P_1"},
    {ASSOCIATIVE_P_1, "ASSOCIATIVE_P_1"},
    {SORTABLE_P_1, "SORTABLE_P_1"},
    {STREAM_P_1, "STREAM_P_1"},
    {INSTREAM_P_1, "INSTREAM_P_1"},
    {OUTSTREAM_P_1, "OUTSTREAM_P_1"},
    {IOSTREAM_P_1, "IOSTREAM_P_1"},
    {SET_P_1, "SET_P_1"},
    {ERROR_P_1, "ERROR_P_1"},
    {IO_ERROR_P_1, "IO_ERROR_P_1"},
    {BOOL_P_1, "BOOL_P_1"},
    {CFN_P_1, "CFN_P_1"},
    {CHARACTER_P_1, "CHARACTER_P_1"},
    {CLOSURE_P_1, "CLOSURE_P_1"},
    {FLOAT_P_1, "FLOAT_P_1"},
    {FN_P_1, "FN_P_1"},
    {FSTREAM_P_1, "FSTREAM_P_1"},
    {HASHMAP_P_1, "HASHMAP_P_1"},
    {HASHSET_P_1, "HASHSET_P_1"},
    {INTEGER_P_1, "INTEGER_P_1"},
    {KEYWORD_P_1, "KEYWORD_P_1"},
    {LIST_P_1, "LIST_P_1"},
    {MAP_P_1, "MAP_P_1"},
    {MAPENTRY_P_1, "MAPENTRY_P_1"},
    {NAMESPACE_P_1, "NAMESPACE_P_1"},
    {PROC_P_1, "PROC_P_1"},
    {REGEX_P_1, "REGEX_P_1"},
    {SSTREAM_P_1, "SSTREAM_P_1"},
    {STRING_P_1, "STRING_P_1"},
    {SYMBOL_P_1, "SYMBOL_P_1"},
    {TREEMAP_P_1, "TREEMAP_P_1"},
    {TREESET_P_1, "TREESET_P_1"},
    {VAR_P_1, "VAR_P_1"},
    {VECTOR_P_1, "VECTOR_P_1"},
    {INT_1, "INT_1"},
    {FLOAT_1, "FLOAT_1"},
    {ADD_0, "ADD_0"},
    {ADD_1, "ADD_1"},
    {ADD_2, "ADD_2"},
    {ADD_2N, "ADD_2N"},
    {SUB_1, "SUB_1"},
    {SUB_2, "SUB_2"},
    {SUB_2N, "SUB_2N"},
    {MUL_0, "MUL_0"},
    {MUL_1, "MUL_1"},
    {MUL_2, "MUL_2"},
    {MUL_2N, "MUL_2N"},
    {DIV_1, "DIV_1"},
    {DIV_2, "DIV_2"},
    {DIV_2N, "DIV_2N"},
    {EQEQ_1, "EQEQ_1"},
    {EQEQ_2, "EQEQ_2"},
    {EQEQ_2N, "EQEQ_2N"},
    {LT_1, "LT_1"},
    {LT_2, "LT_2"},
    {LT_2N, "LT_2N"},
};

int disOne(const FnMethod* m, int addr, std::ostream& s) {
    int opcode = m->bc()[addr];
    s << std::setw(4) << std::hex << std::setfill('0')
      << addr++ << " " << opcodeMap[opcode].name;
    int n = opcodeMap[opcode].operandCount;
    if (n) {
        // if the opcode has operands, print dots up to the 21st column
        int j = 20 - std::strlen(opcodeMap[opcode].name);
        while (j--)
            s << '.';
    }
    switch (n) {
        case 0:
            // no operands
            switch (opcode) {
                // HH HH ... HH
                case NEW_CLOSURE: {
                    int n = m->bc()[addr++];
                    s << ' ' << std::setw(2) << std::hex << std::setfill('0')
                      << n;
                    for (int j=0; j<n; ++j) {
                        s << ' ' << std::setw(2) << std::hex
                          << std::setfill('0') << (int)m->bc()[addr++]
                          << ' ' << std::setw(2) << std::hex
                          << std::setfill('0') << (int)m->bc()[addr++];
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        case 1: {
            // 1 operand
            switch (opcodeMap[opcode].type) {
                case NONE: break;
                case U8: {
                    int x = m->bc()[addr++];
                    s << std::setw(2) << std::hex << std::setfill('0') << x;
                    s << std::dec << " (" << x << ")";
                    break;
                }
                case U16: {
                    int x = m->bc()[addr] | m->bc()[addr + 1] << 8;
                    s << std::setw(4) << std::hex << std::setfill('0') << x;
                    std::string name = "";
                    auto itr = procNames.find(x);
                    if (itr != procNames.end())
                        name = itr->second;
                    // print the procID name, if found in the procNames map-.
                    //                                     .----------------'
                    //                                     v
                    s << std::dec << " (" << x << ") " << name;
                    addr += 2;
                    break;
                }
            }
            break;
        }
        default:
            assert(0 && "UNHANDLED OPCODE OPERAND COUNT");
    }
    s << std::endl;
    return addr;
}

void dis(const FnMethod* m, std::ostream& s) {
    int i=0, j=m->bc().size();
    assert(i <= j);
    while (i != j)
        i = disOne(m, i, s);
}

} // end namespace vasm
