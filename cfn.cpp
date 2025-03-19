/*
  cfn.cpp
  S. Edward Dolan
  Wednesday, June 28 2023
*/

#include "sxp.hpp"

CFn* CFn::create(const std::string& name, Obj*(*f)(Obj**)) {
    return new CFn(name, f);
}

/*
CFn* CFn::create(const std::string& name, std::function<Obj*(Obj**)> lambda) {
    CFn* cfn = new CFn(name, nullptr);
    cfn->_lambda = lambda;
    return cfn;
}
*/

CFn::CFn(const std::string& name, cfn_t cfn)
    : Fn(name),
      _cfn(cfn) /*,
      _lambda()*/ {
    _typeName = "SxCFn";
}

std::string CFn::typeName() {
    return _typeName;
}

std::string CFn::toString() {
    std::stringstream ss;
    ss << "#<" << typeName() << ' ' << _name << ' ' << this << '>';
    return ss.str();
}

FnMethod* CFn::addMethod(int reqArgs) {
    FnMethod* m = Fn::addMethod(false, reqArgs);
    m->nLocals(1 + reqArgs);
    m->appendByte(vasm::CALL_CFN);
    m->appendByte(vasm::RETURN);
    return m;
}

// =========================================================================
// Intern some C functions!

/*
  p points to the address of the cfn on the calling VM's pstack --.
       .----------------------------------------------------------'
       v
  ... cfn arg1 arg2 ... argN]
*/
#define ARG(n) (*(p + n))

#define MAKCFN(name, f, nArgs, params, doc)                     \
    cfn = CFn::create(name, f);                                 \
    Var::intern(ns, Symbol::create(name), cfn)                  \
    ->withMeta(Hashmap::create({                                \
                rt::KW_DOC,                                     \
                String::create(doc),                            \
                rt::KW_PARAMS,                                  \
                String::create("(" + std::string(params) + ")") \
            }));                                                \
    cfn->addMethod(nArgs)

#define MAKCONST(name, val, doc)                                        \
    Var::intern(ns, Symbol::create(name), val)                          \
        ->withMeta(Hashmap::create({                                    \
                    rt::KW_DOC,                                         \
                    String::create(doc)                                 \
                }))

// =========================================================================
// Namespace math

// floating point function 1 arg, with errno check
#define FFN1(fn)                                        \
    errno = 0;                                          \
    double x = fn(cpINumber(ARG(1))->toFloat());        \
    if (errno)                                          \
        throw SxArithmeticError(std::strerror(errno));  \
    return Float::create(x)

// floating point function 2 arg, with errno check
#define FFN2(fn)                                                \
    errno = 0;                                                  \
    double x = fn(cpINumber(ARG(1))->toFloat(),                 \
              cpINumber(ARG(2))->toFloat());                    \
    if (errno)                                                  \
        throw SxError(std::strerror(errno));                    \
    return Float::create(x)

// integer function 1 arg, with errno check
#define IFN1(fn)                                                \
    errno = 0;                                                  \
    long x = fn(cpINumber(ARG(1))->toInt());                    \
    if (errno)                                                  \
        throw SxArithmeticError(std::strerror(errno));          \
    return Integer::fetch(x)

static Obj* c_radians(Obj** p) {
    return Float::create(M_PI / 180.0 * cpINumber(ARG(1))->toFloat());
}
static Obj* c_degrees(Obj** p) {
    return Float::create(180.0 / M_PI * cpINumber(ARG(1))->toFloat());
}
// 1 arg
static Obj* c_sin(Obj** p) { FFN1(std::sin); }
static Obj* c_cos(Obj** p) { FFN1(std::cos); }
static Obj* c_tan(Obj** p) { FFN1(std::tan); }
static Obj* c_asin(Obj** p) { FFN1(std::asin); }
static Obj* c_acos(Obj** p) { FFN1(std::acos); }
static Obj* c_atan(Obj** p) { FFN1(std::atan); }
static Obj* c_sinh(Obj** p) { FFN1(std::sinh); }
static Obj* c_cosh(Obj** p) { FFN1(std::cosh); }
static Obj* c_tanh(Obj** p) { FFN1(std::tanh); }
static Obj* c_asinh(Obj** p) { FFN1(std::asinh); }
static Obj* c_acosh(Obj** p) { FFN1(std::acosh); }
static Obj* c_atanh(Obj** p) { FFN1(std::atanh); }
static Obj* c_exp(Obj** p) { FFN1(std::exp); }
static Obj* c_log(Obj** p) { FFN1(std::log); }
static Obj* c_log10(Obj** p) { FFN1(std::log10); }
static Obj* c_sqrt(Obj** p) { FFN1(std::sqrt); }
static Obj* c_ceil(Obj** p) { FFN1(std::ceil); }
static Obj* c_floor(Obj** p) { FFN1(std::floor); }
static Obj* c_abs(Obj** p) { IFN1(std::abs); }
static Obj* c_fabs(Obj** p) { FFN1(std::fabs); }
// 2 arg
static Obj* c_pow(Obj** p) { FFN2(std::pow); }
static Obj* c_atan2(Obj** p) { FFN2(std::atan2); }
static Obj* c_fmod(Obj** p) { FFN2(std::fmod); }
// 
static Obj* c_modf(Obj** p) {
    double i = 0.0;
    double f = std::modf(cpINumber(ARG(1))->toFloat(), &i);
    return Vector::create({
            Integer::fetch(i),
            Float::create(f)
        });
}
// like x % y, integers only, clojure permits any number type
static Obj* c_mod(Obj** p) {
    return Integer::fetch(cpInteger(ARG(1))->val() %
                          cpInteger(ARG(2))->val());
}
void initMath() {
    CFn* cfn = nullptr;
    Namespace* ns = Namespace::fetch(Symbol::create("math"));
    MAKCFN("radians", c_radians, 1, "[x]", "Convert x degrees to radians.");
    MAKCFN("degrees", c_degrees, 1, "[x]", "Convert x radians to degrees.");
    MAKCFN("sin", c_sin, 1, "[x]", "Return the sin of x, given in radians.");
    MAKCFN("cos", c_cos, 1, "[x]", "");
    MAKCFN("tan", c_tan, 1, "[x]", "");
    MAKCFN("asin", c_asin, 1, "[x]", "");
    MAKCFN("acos", c_acos, 1, "[x]", "");
    MAKCFN("atan", c_atan, 1, "[x]", "");
    MAKCFN("sinh", c_sinh, 1, "[x]", "");
    MAKCFN("cosh", c_cosh, 1, "[x]", "");
    MAKCFN("tanh", c_tanh, 1, "[x]", "");
    MAKCFN("asinh", c_asinh, 1, "[x]", "");
    MAKCFN("acosh", c_acosh, 1, "[x]", "");
    MAKCFN("atanh", c_atanh, 1, "[x]", "");
    MAKCFN("exp", c_exp, 1, "[x]", "");
    MAKCFN("log", c_log, 1, "[x]", "");
    MAKCFN("log10", c_log10, 1, "[x]", "");
    MAKCFN("sqrt", c_sqrt, 1, "[x]", "");
    MAKCFN("ceil", c_ceil, 1, "[x]", "");
    MAKCFN("floor", c_floor, 1, "[x]", "");
    MAKCFN("abs", c_abs, 1, "[x]", "");
    MAKCFN("fabs", c_fabs, 1, "[x]", "");
    MAKCFN("pow", c_pow, 2, "[x y]", "Return x raised to the power of y.");
    MAKCFN("atan2", c_atan2, 2, "[y x]", "Return the arc tangent of y/x.");
    MAKCFN("fmod", c_fmod, 2, "[x y]", "");
    MAKCFN("modf", c_modf, 1, "[x]", "Return a vector containing the"
           " intergral and fractional parts of x.");
    MAKCFN("mod", c_mod, 2, "[x y]", "Return modulo of the integers x and y.");
    // math `constants'
    MAKCONST("E", Float::create(M_E), "The mathematical constant e.");
    MAKCONST("PI", Float::create(M_PI), "The mathematical constant pi.");
    MAKCONST("SQRT2", Float::create(M_SQRT2), "The constant sqrt(2).");
    MAKCONST("PI-2", Float::create(M_PI_2), "The constant pi/2.");
}

// =========================================================================
// Namespace str

static Obj* c_reverse(Obj** p) {
    std::string s(cpString(ARG(1))->val());
    std::reverse(s.begin(), s.end());
    return String::fetch(s);
}
static std::string& c_ltrimHelper(std::string& s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int> (std::isspace))));
    return s;
}
static std::string& c_rtrimHelper(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>
                                   (std::isspace))).base(),
            s.end());
    return s;
}
static Obj* c_ltrim(Obj** p) {
    std::string s(cpString(ARG(1))->val());
    return String::fetch(c_ltrimHelper(s));
}
static Obj* c_rtrim(Obj** p) {
    std::string s(cpString(ARG(1))->val());
    return String::fetch(c_rtrimHelper(s));
}
static Obj* c_trim(Obj** p) {
    std::string s(cpString(ARG(1))->val());
    return String::fetch(c_rtrimHelper(c_ltrimHelper(s)));
}
static Obj* c_blank_p(Obj** p) {
    if (ARG(1) == NIL)
        return rt::T;
    String* s = cpString(ARG(1));
    if (s->isEmpty())
        return rt::T;
    const std::string& str = s->val();
    if (std::count_if(str.begin(), str.end(),
                      [](unsigned char c){ return !std::isspace(c);}))
        return rt::F;
    return rt::T;
}
static Obj* c_capitalize(Obj** p) {
    std::string s(cpString(ARG(1))->val());
    if (!s.empty() && std::isalpha(s[0]))
        s[0] = std::toupper(s[0]);
    return String::fetch(s);
}
static Obj* c_endsWith(Obj** p) {
    const std::string& x = cpString(ARG(1))->val();
    const std::string& y = cpString(ARG(2))->val();
    return x.size() >= y.size()
        && !x.compare(x.size() - y.size(), y.size(), y)
        ? rt::T
        : rt::F;
}
static Obj* c_startsWith(Obj** p) {
    const std::string& x = cpString(ARG(1))->val();
    const std::string& y = cpString(ARG(2))->val();
    return x.size() >= y.size()
        && !x.compare(0, y.size(), y)
        ? rt::T
        : rt::F;
}
static Obj* c_upcase(Obj** p) {
    std::string s = cpString(ARG(1))->val();
    std::string out(s.size(), ' ');
    std::transform(s.begin(), s.end(), out.begin(), toupper);
    return String::fetch(out);
}
static Obj* c_downcase(Obj** p) {
    std::string s = cpString(ARG(1))->val();
    std::string out(s.size(), ' ');
    std::transform(s.begin(), s.end(), out.begin(), tolower);
    return String::fetch(out);
}

static void initStr() {
    CFn* cfn = nullptr;
    Namespace* ns = Namespace::fetch(Symbol::create("str"));
    MAKCFN("blank?", c_blank_p, 1, "[s]", "Return true if s is nil, empty, or"
           " contains only whitespace.");
    MAKCFN("capitalize", c_capitalize, 1, "[s]", "Return a copy of s with the"
           " first character capitalized (if a letter).");
    MAKCFN("reverse", c_reverse, 1, "[s]", "Return a copy of s with the"
           " characters reversed.");
    MAKCFN("ltrim", c_ltrim, 1, "[s]", "Return a copy of s with all whitespace"
           " removed from the beginning of the string.");
    MAKCFN("rtrim", c_rtrim, 1, "[s]", "Return a copy of s with all whitespace"
           " removed from the end of the string.");
    MAKCFN("trim", c_trim, 1, "[s]", "Return a copy of s with all whitespace"
           " removed from both ends of the string.");
    MAKCFN("ends-with", c_endsWith, 2, "[s suffix]", "Return true if s ends"
           " with the given suffix, case-sensitive.");
    MAKCFN("starts-with", c_startsWith, 2, "[s prefix]", "Return true if s"
           " starts with the given prefix, case-sensitive.");
    MAKCFN("upcase", c_upcase, 1, "[s]", "Return a copy of s converting all"
           " aplha charcters to upper case.");
    MAKCFN("downcase", c_downcase, 1, "[s]", "Return a copy of s converting"
           " all aplha charcters to lower case.");
}

// =========================================================================

void CFn::initCFns() {
    initMath();
    initStr();
}

#undef FFN1
#undef FFN2
#undef IFN1
#undef ARG
#undef MAKCFN
