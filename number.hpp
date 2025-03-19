/*
  number.hpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#ifndef NUMBER_HPP_INCLUDED
#define NUMBER_HPP_INCLUDED

struct Integer : INumber, ISortable {
    static void init();
    static void shutdown();
    static Integer* fetch(long);
    long val();
    // Obj
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Integer* copy() { return this; }
    // ISortable
    bool less(Obj*);
    // INumber
    long toInt() { return _val; }
    double toFloat() { return _val; }
    INumber* add(INumber*);
    INumber* sub(INumber*);
    INumber* mul(INumber*);
    INumber* div(INumber*);
    INumber* neg();
    bool eq(INumber*);
    bool lt(INumber*);
protected:
    static constexpr int MIN_CACHED_INT = -128;
    static constexpr int MAX_CACHED_INT = 512;
    static std::vector<Integer*, gc_allocator<Integer*>> _cache;
    long _val;
    size_t _hash;
    Integer() {}
    Integer(long);
};
DEF_CASTER(Integer)

struct Float : INumber, ISortable {
    static Float* create(double val);
    double val();
    // Obj
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Float* copy() { return this; }
    // ISortable
    bool less(Obj*);
    // INumber
    long toInt() { return _val; }
    double toFloat() { return _val; }
    INumber* add(INumber*);
    INumber* sub(INumber*);
    INumber* mul(INumber*);
    INumber* div(INumber*);
    INumber* neg();
    bool eq(INumber*);
    bool lt(INumber*);
protected:
    double _val;
    size_t _hash;
    Float();
    Float(double);
};
DEF_CASTER(Float)

struct Ratio : INumber, ISortable {
    static INumber* simplify(long, long);
    long numerator();
    long denominator();
    std::string toString();
    size_t getHash();
    bool isEqualTo(Obj*);
    Ratio* copy() { return this; }
    // ISortable
    bool less(Obj*);
    // INumber
    long toInt();
    double toFloat();
    INumber* add(INumber*);
    INumber* sub(INumber*);
    INumber* mul(INumber*);
    INumber* div(INumber*);
    INumber* neg();
    bool eq(INumber*);
    bool lt(INumber*);
protected:
    long _num;
    long _den;
    size_t _hash;
    Ratio(long num, long den);
};
DEF_CASTER(Ratio)

#endif // NUMBER_HPP_INCLUDED
