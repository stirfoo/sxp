/*
  regex.cpp
  S. Edward Dolan
  Monday, June 26 2023
*/

#include "sxp.hpp"

Regex* Regex::create(const std::string& pattern,
                     std::regex::flag_type flags) {
    try {
        std::regex re(pattern, flags);
        return new (PointerFreeGC) Regex(re, pattern);
    }
    catch (std::regex_error& e) {
        throw SxRegexError(e.what());
    }
}

Regex::Regex(const std::regex& re, const std::string& pattern)
    : _re(re),
      _pat(pattern) {
    _typeName = "SxRegex";
}

std::string Regex::toString() {
    std::stringstream ss;
    ss << "#<" << typeName() << " pat=" << _pat << ' ' << this << '>';
    return ss.str();
}
