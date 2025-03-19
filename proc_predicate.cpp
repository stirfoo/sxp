/*
  proc_predicate.cpp
  S. Edward Dolan
  Saturday, June 24 2023
*/

// =========================================================================
// Interface Predicates

// ... proc x]
// ... proc x bool]
case vasm::SEQ_P_1: {
    ppush(pISeq(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::SEQABLE_P_1: {
    ppush(pISeqable(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::NUMBER_P_1: {
    ppush(pINumber(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::INDEXED_P_1: {
    ppush(pIIndexed(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::COLLECTION_P_1: {
    ppush(pICollection(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::ASSOCIATIVE_P_1: {
    ppush(pIAssociative(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::SORTABLE_P_1: {
    ppush(pISortable(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::STREAM_P_1: {
    ppush(pIStream(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::INSTREAM_P_1: {
    ppush(pIInStream(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::OUTSTREAM_P_1: {
    ppush(pIOutStream(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::SET_P_1: {
    ppush((pHashset(ppeek()) || pTreeset(ppeek())) ? rt::T : rt::F);
    break;
}

// =========================================================================
// Error Type Predicates

// ... proc x]
// ... proc x bool]
case vasm::ERROR_P_1: {
    ppush(pSxError(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::IO_ERROR_P_1: {
    ppush(pSxIOError(ppeek()) ? rt::T : rt::F);
    break;
}

// =========================================================================
// Other Type Predicates

// ... proc x]
// ... proc x bool]
case vasm::BOOL_P_1: {
    ppush(pBool(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::CFN_P_1: {
    ppush(pCFn(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::CHARACTER_P_1: {
    ppush(pCharacter(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::CLOSURE_P_1: {
    ppush(pClosure(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::FLOAT_P_1: {
    ppush(pFloat(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::FN_P_1: {
    ppush(pFn(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::FSTREAM_P_1: {
    ppush(pFStream(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::HASHMAP_P_1: {
    ppush(pHashmap(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::HASHSET_P_1: {
    ppush(pHashset(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::INTEGER_P_1: {
    ppush(pInteger(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::KEYWORD_P_1: {
    ppush(pKeyword(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::LIST_P_1: {
    ppush(pList(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::MAP_P_1: {
    ppush((pHashmap(ppeek()) || pTreemap(ppeek())) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::MAPENTRY_P_1: {
    ppush(pMapEntry(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::NAMESPACE_P_1: {
    ppush(pNamespace(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::PROC_P_1: {
    ppush(pProc(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::REGEX_P_1: {
    ppush(pRegex(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::SSTREAM_P_1: {
    ppush(pSStream(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::STRING_P_1: {
    ppush(pString(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::SYMBOL_P_1: {
    ppush(pSymbol(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::TREEMAP_P_1: {
    ppush(pTreemap(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::TREESET_P_1: {
    ppush(pTreeset(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::VAR_P_1: {
    ppush(pVar(ppeek()) ? rt::T : rt::F);
    break;
}
// ... proc x]
// ... proc x bool]
case vasm::VECTOR_P_1: {
    ppush((pVector(ppeek()) || pMapEntry(ppeek())) ? rt::T : rt::F);
    break;
}

