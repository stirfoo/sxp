/*
  proc_error.cpp
  S. Edward Dolan
  Saturday, June 24 2023
*/

// ... proc]
// ... proc e]
case vasm::ERROR_0: {
    ppush(new (PointerFreeGC) SxError());
    break;
}
// ... proc string]
// ... proc string e]
case vasm::ERROR_1: {
    ppush(new (PointerFreeGC) SxError(cpString(ppeek())->val()));
    break;
}
// ... proc type string]
// ... proc type string e]
case vasm::ERROR_2: {
    ppush(cpSxError(ppeek(1))->clone(cpString(ppeek())->val()));
    break;
}
// ... proc e]
// ... proc e string]
case vasm::ERR_MSG_1: {
    ppush(String::fetch(cpSxError(ppeek())->what()));
    break;
}
