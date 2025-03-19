/*
  keyword.hpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#ifndef KEYWORD_HPP_INCLUDED
#define KEYWORD_HPP_INCLUDED

/*
  Can be a function:
  (:foo {:one 1 :foo 2})                   => 2
  (:foo {:one 1 :foo 2} :three)            => nil
  (:foo {:one 1 :foo 2} :three :not-found) => :not-found
 */
struct Keyword : Fn, ISortable {
    friend struct WeakRefMap<Keyword>;
    static void shutdown() { _cache.clear(); }
    static Keyword* fetch(const std::string&);
    static void dumpCache();
    //
    std::string toString();
    size_t getHash();
    Keyword* copy() { return this; }
    // NOTE: Keywords are cached and compared by pointer by Obj::isEqualTo
    // bool isEqualTo(Obj*);
    //
    bool less(Obj*);
protected:    
    std::string _name;
    size_t _hash;
    static WeakRefMap<Keyword> _cache;
    void createMethods();
    Keyword(const std::string& name);
};
DEF_CASTER(Keyword)

#endif // KEYWORD_HPP_INCLUDED
