/*
  sxp.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef SXP_HPP_INCLUDED
#define SXP_HPP_INCLUDED

#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

#include <iostream>
#include <fstream>              // fstream impl
#include <sstream>              // sstream impl
#include <string>               // string impl
#include <vector>               // vector impl
#include <map>                  // treemap impl (ordered)
#include <set>                  // treeset impl (ordered)
#include <unordered_map>        // hashmap impl
#include <unordered_set>        // hashset impl
#include <stdexcept>            // error impl
#include <functional>
#include <iomanip>
#include <limits>
#define _USE_MATH_DEFINES       // TODO: create my own math constants
#include <cmath>
#include <cassert>
#include <cstring>
#include <numeric>
#include <regex>                // regex impl

#include "obj.hpp"
#include "error.hpp"
#include "xface.hpp"
#include "bool.hpp"
#include "number.hpp"
#include "weakrefmap.hpp"
#include "fn.hpp"
#include "str.hpp"
#include "char.hpp"
#include "keyword.hpp"
#include "symbol.hpp"
#include "regex.hpp"
#include "list.hpp"
#include "vector.hpp"
#include "mapentry.hpp"
#include "hashmap.hpp"
#include "hashset.hpp"
#include "treemap.hpp"
#include "treeset.hpp"
#include "lazyseq.hpp"
#include "proc.hpp"
#include "cfn.hpp"
#include "stream.hpp"
#include "var.hpp"
#include "namespace.hpp"
#include "vm.hpp"
#include "rt.hpp"
#include "reader.hpp"
#include "compiler.hpp"
#include "vasm.hpp"

#endif // SXP_HPP_INCLUDED
