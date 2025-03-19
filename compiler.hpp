/*
  compiler.hpp
  S. Edward Dolan
  Saturday, May 13 2023
*/

#ifndef COMPILER_HPP_INCLUDED
#define COMPILER_HPP_INCLUDED

namespace compiler {

Fn* compile(Obj*);
Symbol* resolveSymbol(Symbol*);
Obj* macroExpand1(Obj* form, bool initialize=false);

}

#endif // COMPILER_HPP_INCLUDED
