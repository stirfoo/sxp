/*
  number.cpp
  S. Edward Dolan
  Sunday, April  9 2023
*/

#include "sxp.hpp"

// =========================================================================
// Integer

std::vector<Integer*, gc_allocator<Integer*>> Integer::_cache;

void Integer::init() {
    _cache.resize(MAX_CACHED_INT - MIN_CACHED_INT + 1);
}

void Integer::shutdown() {
    _cache.clear();
}

Integer* Integer::fetch(long val) {
    if (val >= MIN_CACHED_INT && val <= MAX_CACHED_INT) {
        int idx = val + std::abs(MIN_CACHED_INT);
        if (Integer* obj = _cache[idx])
            return obj;
        else
            return _cache[idx] = new (PointerFreeGC) Integer(val);
    }
    return new (PointerFreeGC) Integer(val);
}

Integer::Integer(long val)
    : _val(val), _hash(std::hash<long>()(val)) {
    _typeName = "SxInteger";
}

long Integer::val() {
    return _val;
}

std::string Integer::toString() {
    std::stringstream ss;
    ss << _val;
    return ss.str();
}

size_t Integer::getHash() {
    return _hash;
}

bool Integer::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Integer* i = dynamic_cast<Integer*>(obj))
        return _val == i->_val;
    return false;
}

bool Integer::less(Obj* x) {
    return toFloat() < cpINumber(x)->toFloat();
}

// -------------------------------------------------------------------------
// INumber

INumber* Integer::add(INumber* y) {
    if (Integer* p = pInteger(y))
        return Integer::fetch(_val + p->val());
    else if (Float* p = pFloat(y))
        return Float::create(_val + p->val());
    else {
        Ratio* r = pRatio(y);
        long n = r->numerator();
        long d = r->denominator();
        if (n < 0)
            return Ratio::simplify(_val * d - -n, d);
        else
            return Ratio::simplify(_val * d + n, d);
    }
}

INumber* Integer::sub(INumber* y) {
    if (Integer* p = pInteger(y))
        return Integer::fetch(_val - p->val());
    else if (Float* p = pFloat(y))
        return Float::create(_val - p->val());
    else  {
        Ratio* r = pRatio(y);
        long n = r->numerator();
        long d = r->denominator();
        if (n < 0)
            return Ratio::simplify(_val * d + -n, d);
        else
            return Ratio::simplify(_val * d - n, d);
    }
}

INumber* Integer::mul(INumber* y) {
    if (Integer* p = pInteger(y))
        return Integer::fetch(_val * p->val());
    else if (Float* p = pFloat(y))
        return Float::create(_val * p->val());
    else {
        Ratio* r = pRatio(y);
        return Ratio::simplify(_val * r->numerator(), r->denominator());
    }
}

INumber* Integer::div(INumber* y) {
    if (y->toFloat() == 0.0)
        throw SxArithmeticError("can't divide by zero");
    if (Integer* p = pInteger(y))
        return Ratio::simplify(_val, p->val());
    else if (Float* p = pFloat(y))
        return Float::create(_val / p->val());
    return mul(Ratio::simplify(pRatio(y)->denominator(), // inverted
                               pRatio(y)->numerator()));
}

INumber* Integer::neg() {
    return Integer::fetch(-val());
}

bool Integer::eq(INumber* x) {
    if (this == x)
        return true;
    return _val == x->toFloat();
}

bool Integer::lt(INumber* x) {
    if (this == x)
        return false;
    return _val < x->toFloat();
}

// =========================================================================
// Float

Float* Float::create(double val) {
    return new (PointerFreeGC) Float(val);
}

double Float::val() {
    return _val;
}

Float::Float(double val)
    : _val(val), _hash(std::hash<double>()(val)) {
    _typeName = "SxFloat";
}

std::string Float::toString() {
    double f;
    std::stringstream ss;
    if (std::modf(_val, &f))
        ss << std::setprecision(std::numeric_limits<double>::digits10)
           << _val;
    else
        ss << _val << ".0";     // 42 => 42.0
    return ss.str();
}

size_t Float::getHash() {
    return _hash;
}

bool Float::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    if (Float* i = pFloat(obj))
        return _val == i->_val;
    return false;
}

bool Float::less(Obj* x) {
    return toFloat() < cpINumber(x)->toFloat();
}

// -------------------------------------------------------------------------
// INumber

INumber* Float::add(INumber* y) {
    return Float::create(_val + y->toFloat());
}

INumber* Float::sub(INumber* y) {
    return Float::create(_val - y->toFloat());
}

INumber* Float::mul(INumber* y) {
    return Float::create(_val * y->toFloat());
}

INumber* Float::div(INumber* y) {
    double yy = y->toFloat();
    if (yy == 0.0)
        throw SxArithmeticError("can't divide by zero");
    return Float::create(_val / yy);
}

INumber* Float::neg() {
    return Float::create(-val());
}

bool Float::eq(INumber* x) {
    if (this == x)
        return true;
    return _val == x->toFloat();
}

bool Float::lt(INumber* x) {
    if (this == x)
        return false;
    return _val < x->toFloat();
}

// =========================================================================
// Ratio

INumber* Ratio::simplify(long n, long d) {
    if (d < 0) {
        // invert so only numerator is negative
        assert(n >= 0 && "BOTH NUM AND DEN CANNOT BE NEGATIVE");
        n = -n;
        d = -d;
    }
    if (d == 0)
        throw SxArithmeticError("divide by zero");
    if (n == d)
        return Integer::fetch(1);
    if (d == 1)
        return Integer::fetch(n);
    if (n == 0)
        return Integer::fetch(0);
    if (n % d == 0)
        return Integer::fetch(n / d);
    long gcd = std::gcd<long, long>(n, d);
    if (d == gcd)
        return Integer::fetch(1);
    return new (PointerFreeGC) Ratio(n / gcd, d / gcd);
}

Ratio::Ratio(long num, long den)
    : _num(num),
      _den(den),
      _hash(std::hash<long>()(_num) ^ std::hash<long>()(den)) {
    assert(num != 0 && "RATIO NUMERATOR CANNOT BE 0");
    assert(den != 0 && "RATIO DENOMINATOR CANNOT BE 0");
    assert(den != 1 && "RATIO DENOMINATOR CANNOT BE 1");
    _typeName = "SxRatio";
}

std::string Ratio::toString() {
    std::stringstream ss;
    ss << _num << '/' << _den;
    return ss.str();
}

size_t Ratio::getHash() {
    return _hash;
}

bool Ratio::isEqualTo(Obj* x) {
    if (this == x)
        return true;
    if (Ratio* p = pRatio(x))
        return _num == p->numerator()
            && _den == p->denominator();
    return false;
}

bool Ratio::less(Obj* x) {
    return toFloat() < cpINumber(x)->toFloat();
}

long Ratio::numerator() {
    return _num;
}

long Ratio::denominator() {
    return _den;
}

// -------------------------------------------------------------------------
// INumber

long Ratio::toInt() {
    return _num / _den;
}

double Ratio::toFloat() {
    return static_cast<double>(_num) / static_cast<double>(_den);
}

INumber* Ratio::add(INumber* y) {
    if (Integer* p = pInteger(y))
        return Ratio::simplify(p->val() * _den + _num, _den);
    else if (Float* p = pFloat(y))
        return Float::create(toFloat() + p->val());
    else {
        Ratio* r = pRatio(y);
        int lcm = std::lcm<long, long>(_den, r->denominator());
        long xn = _num * (lcm / _den);
        long yn = r->numerator() * (lcm / r->denominator());
        if (yn < 0)
            return Ratio::simplify(xn - -yn, lcm);
        else
            return Ratio::simplify(xn + yn, lcm);
    }
}

INumber* Ratio::sub(INumber* y) {
    if (Integer* p = pInteger(y))
        return add(Integer::fetch(-p->val()));
    else if (Float* p = pFloat(y))
        return Float::create(toFloat() - p->val());
    else {
        Ratio* r = pRatio(y);
        int lcm = std::lcm<long, long>(_den, r->denominator());
        long xn = _num * (lcm / _den);
        long yn = r->numerator() * (lcm / r->denominator());
        if (yn < 0)
            return Ratio::simplify(xn + -yn, lcm);
        else
            return Ratio::simplify(xn - yn, lcm);
    }
}

INumber* Ratio::mul(INumber* y) {
    if (Integer* p = pInteger(y))
        return simplify(_num * p->val(), _den);
    else if (Float* p = pFloat(y)) {
        return Float::create(toFloat() * p->val());
    }
    else {
        Ratio* r = pRatio(y);
        long xn = _num * r->numerator();
        long yn = _den * r->denominator();
        return Ratio::simplify(xn, yn);;
    }
}

INumber* Ratio::div(INumber* y) {
    if (y->toFloat() == 0.0)
        throw SxArithmeticError("can't divide by zero");
    else if (Float* p = pFloat(y))
        return Float::create(toFloat())->div(p);
    else {
        long n, d;
        if (Integer* p = pInteger(y)) {
            n = p->val();
            d = 1;
        }
        else {
            n = pRatio(y)->numerator();
            d = pRatio(y)->denominator();
        }
        long xn, yn;
        if (n < 0) {
            xn = _num * -d;
            yn = _den * -n;
        }
        else {
            xn = _num * d;
            yn = _den * n;
        }
        return Ratio::simplify(xn, yn);
    }
}

INumber* Ratio::neg() {
    return Ratio::simplify(-_num, _den);
}

bool Ratio::eq(INumber* x) {
    if (this == x)
        return true;
    return toFloat() == x->toFloat();
}

bool Ratio::lt(INumber* x) {
    if (this == x)
        return false;
    return toFloat() < x->toFloat();
}
