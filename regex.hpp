/*
  regex.hpp
  S. Edward Dolan
  Saturday, June 17 2023
*/

#ifndef REGEX_HPP_INCLUDED
#define REGEX_HPP_INCLUDED

struct Regex : Obj {
    static Regex* create(const std::string& pattern,
                         std::regex::flag_type flags
                         =std::regex_constants::ECMAScript);
    std::string toString();
    Regex* copy() { return this; }
    const std::string& pat() { return _pat; }
    const std::regex& re() { return _re; }
protected:
    std::regex _re;
    std::string _pat;
    Regex(const std::regex& re, const std::string& pattern);
};
DEF_CASTER(Regex)

#endif // REGEX_HPP_INCLUDED
