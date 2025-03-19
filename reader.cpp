/*
  reader.cpp
  S. Edward Dolan
  Sunday, November 29, 2020

  Some of this code is from the Clojure 1.2 source tree. Most especially the
  quasi-quote and fn-short-hand code.
  
  Ignoring all whitespace and commas
  
  Reading
  * ; comment
  * symbol
  * :keyword
  * 42, 0xff, 0777 integers
  * 3.14, .1, 1., 1e2,  3e-3 1.e+2 .1e+2, etc. floats
  * 3/4, -4/9, etc. ratios
  * (...) list
  * [...] vector
  * {...} hashmap
  * #{...} hashset
  * 'quote
  * #_ skip next expr
  * #<unreadable>
  * "string"
  * \x character literal
  * ~ unquote
  * ~@ unquote-splicing
  * ` quasi-quote
  * foo# auto-gensyms (within a quasiquote form)
  * #'foo var
  * #^{...}obj, meta data
  * #(...) function shorthand
  * % %N %& fn shorthand parameters
  * #/pattern/opts regex               TODO: the opts part
 */

#include "sxp.hpp"

namespace reader {

typedef Obj*(*readFn_t)(IInStream*, int);
typedef std::map<int, readFn_t> mapChFn_t;

static mapChFn_t initMacroMap();
static mapChFn_t macroMap = initMacroMap();
static mapChFn_t initDispatchMacroMap();
static mapChFn_t dispatchMacroMap = initDispatchMacroMap();

static Obj* read(IInStream*, bool, Obj* eofObj = NIL);

static bool isMacroChar(int c) {
    return macroMap.find(c) != macroMap.end();
}

static bool isTerminatingMacroChar(int c) {
    return c != '#' && isMacroChar(c);
}

static bool isWhitespace(int c) {
    return isspace(c) || ',' == c;
}

static INumber* matchNumber(const std::string& s) {
    char* p;
    errno = 0;
    long lx = std::strtol(s.c_str(), &p, 0);
    if (*p == '/') {
        // maybe a ratio
        bool gotBadSign = false;
        if (std::strchr("+-", *(p + 1)))
            gotBadSign = true;
        bool numOverflow = false;
        if (errno == ERANGE)
            numOverflow = true;
        long num = lx;
        char* d = ++p;
        errno = 0;
        long den = std::strtol(d, &p, 0);
        if (*p == 0) {
            if (gotBadSign)
                throw SxReaderError("ratio denominator cannot be signed: "
                                    + s);
            if (numOverflow)
                throw SxReaderError("literal ratio numerator under/overflow: "
                                    + s);
            if (errno == ERANGE)
                throw SxReaderError("literal ratio denominator"
                                    " under/overflow: " + s);
            return Ratio::simplify(num, den);
        }
    }
    else if (*p == 0) {
        if (errno == ERANGE)
            throw SxReaderError("integer under/over flow: " + s);
        return Integer::fetch(lx);
    }
    errno = 0;
    double dx = std::strtod(s.c_str(), &p);
    if (errno == ERANGE)
        throw SxReaderError("floating point under/over flow: " + s);
    else if (*p == '\0')
        return Float::create(dx);
    return nullptr;
}

// literal integer or float => INumber
static INumber* readNumber(IInStream* stream, int c) {
    std::string buf;
    buf += c;
    while (true) {
        c = stream->get();
        if (stream->eof() || isWhitespace(c) || isMacroChar(c)) {
            stream->unget();
            break;
        }
        buf += c;
    }
    INumber* num = matchNumber(buf);
    if (!num)
        throw SxReaderError("invalid number: " + buf);
    return num;
}

// ; ... => slurp til end of line and return the stream
static Obj* readComment(IInStream* stream, int c) {
    do {
        c = stream->get();
    } while (!stream->eof() && ('\n' != c && '\r' != c));
    return stream;
}

// 'foo => (quote foo)
static Obj* readQuote(IInStream* stream, int c) {
    (void)c;                    // c is '
    Obj* x = readOne(stream, true);
    return List::create(x)->cons(rt::SYM_QUOTE);
}

static std::string readToken(IInStream* stream, int c) {
    std::string buf;
    buf += c;
    while (true) {
        c = stream->get();
        if (stream->eof() || isWhitespace(c) || isTerminatingMacroChar(c)) {
            stream->unget();
            return buf;
        }
        buf += c;
    }
}

static Obj* matchSymbol(const std::string& s) {
    size_t n = s.size();
    if (n > 1 && s[0] == ':')
        return Keyword::fetch(s.substr(1));
    return Symbol::create(s);
}

static Obj* interpretToken(const std::string& s) {
    if ("nil" == s)
        return nullptr;
    if ("true" == s)
        return rt::T;
    if ("false" == s)
        return rt::F;
    if ("/" == s)
        return Symbol::create(s);
    return matchSymbol(s);
}

// [0-9A-Za-z][0-9A-Za-z] => int
static int readHexChar(IInStream* stream) {
    int c;
    char buf[3] = {0};
    c = stream->get();
    if (stream->eof())
        throw SxReaderError("end of input expecting a hex digit");
    if (!isxdigit(c))
        throw SxReaderError("expected a hex digit");
    buf[0] = c;
    c = stream->get();
    if (stream->eof() || !isxdigit(c))
        stream->unget();
    else
        buf[1] = c;
    int x = strtol(buf, nullptr, 16);
    if (x > 127)
        throw SxReaderError("hexadecimal escape must be < 128");
    return x;
}

static inline bool isodigit(int c) {
    return c >= '0' && c <= '7';
}

// [0-7]{1, 3} => int
static int readOctalChar(IInStream* stream, int c) {
    char buf[] = {(char)c, 0, 0, 0};
    for (int i=1; i<3; ++i) {
        c = stream->get();
        if (stream->eof())
            break;
        if (!isodigit(c)) {
            stream->unget();
            break;
        }
        buf[i] = c;
    }
    int x = strtol(buf, nullptr, 8);
    if (x > 127)
        throw SxReaderError("octal escape must be < 128");
    return x;
}

// "..."
static Obj* readString(IInStream* stream, int c) {
    std::string buf;
    for (c = stream->get(); c!='"'; c = stream->get()) {
        if (stream->eof())
            throw SxReaderError("EOF while reading string");
        if ('\\' == c) {
            switch (c = stream->get()) {
                case '"': break;
                case '\\': break;
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'x': c = readHexChar(stream); break;
                case 'r': c = '\r'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                case 'a': c = '\a'; break;
                case 'u': c = '\n'; throw SxReaderError("sxp does not support"
                                                        " unicode escapes chars"
                                                        " in literal strings");
                case '\'': break;
                case '?': break;
                default: {
                    if (isodigit(c))
                        c = readOctalChar(stream, c);
                    else
                        throw SxReaderError("unknown escape character");
                    break;
                }
            }
        }
        buf += c;
    }
    if (c != '"')
        throw SxReaderError("end of input expecting string delimiter (\")");
    return String::fetch(buf);
}

// \x \space
static Obj* readCharacter(IInStream* stream, int c) {
    c = stream->get();
    if (stream->eof())
        throw SxReaderError("end of input while reading literal character");
    char* p = nullptr;
    long x = 0;
    std::string tok = readToken(stream, c);
    if (tok.size() > 1) {
        if (tok == "space") c = ' ';
        else if (tok == "tab") c = '\t';
        else if (tok == "newline") c = '\n';
        else if (tok == "return") c = '\r';
        else if (tok == "backspace") c = '\b';
        else if (tok == "formfeed") c = '\f';
        else if (tok == "vtab") c = '\v';
        else if (tok == "bell") c = '\a';
        else if (tok[0] == 'x' || tok[0] == 'X') {
            x = strtol(tok.c_str() + 1, &p, 16);
            if (*p)
                throw SxReaderError("illegal hex escape in character"
                                    " literal: \\" + tok);
            else if (x > 127)
                throw SxReaderError("hexadecimal escape must be < 128, got: \\"
                                    + tok);
            c = (int)x;
        }
        else if (isodigit(tok[0])) {
            x = strtol(tok.c_str() + 1, &p, 8);
            if (*p)
                throw SxReaderError("illegal octal escape in character"
                                    " literal: \\" + tok);
            else if (x > 127)
                throw SxReaderError("octal escape must be < 128, got: \\"
                                    + tok);
            c = (int)x;
        }
        else
            throw SxReaderError("unknown literal character name: \\" + tok);
    }
    return Character::fetch(c);
}

// ) ] }
static Obj* readUnmatchedDelimiter(IInStream* stream, int c) {
    (void)stream;
    std::string msg("unmatched delimiter: ");
    throw SxReaderError(msg + (char)c);
}

// e* delim => vecobj_t
static vecobj_t readDelimitedList(IInStream* stream, int delim) {
    vecobj_t v;
    while (true) {
        int c = stream->get();
        while (isWhitespace(c))
            c = stream->get();
        if (stream->eof())
            throw SxReaderError("EOF while reading");
        if (c == delim)
            break;
        auto itr = macroMap.find(c);
        if (itr != macroMap.end()) {
            Obj* x = macroMap[c](stream, c);
            if (x != stream)
                v.push_back(x);
        }
        else {
            stream->unget();
            v.push_back(read(stream, true));
        }
    }
    return v;
}

// (...) => List
static Obj* readList(IInStream* stream, int c) {
    (void)c;
    return List::create(readDelimitedList(stream, ')'));
}

// [...] => Vector
static Obj* readVector(IInStream* stream, int c) {
    (void)c;
    return Vector::create(readDelimitedList(stream, ']'));
}

// {...} => Hashmap
static Obj* readHashmap(IInStream* stream, int c) {
    (void)c;
    return Hashmap::create(readDelimitedList(stream, '}'));
    // 
    // NOTE: This will handle 'foo as a key and return foo instead of (quote
    //       foo) which is not hashable.
    // 
    // List* lst = nullptr;
    // for (auto& e : v) {
    //     if ((lst = pList(e)) && lst->count() == 2
    //         && lst->first() == rt::SYM_QUOTE)
    //         e = lst->next()->first();
    // }
    //
}

// #{...} => Hashset
static Obj* readHashset(IInStream* stream, int c) {
    (void)c;
    return Hashset::create(readDelimitedList(stream, '}'));
}

// #_ => ignore the next object
static Obj* readDiscard(IInStream* stream, int c) {
    (void)c;
    read(stream, true);
    return stream;
}

// #... => result
static Obj* readDispatch(IInStream* stream, int c) {
    c = stream->get();
    if (stream->eof())
        throw SxReaderError("EOF while reading");
    auto itr = dispatchMacroMap.find(c);
    if (itr != dispatchMacroMap.end())
        return dispatchMacroMap[c](stream, c);
    else {
        std::string msg("no dispatch macro for: ");
        throw SxReaderError(msg + (char)c);
    }
}

// #< => throw
static Obj* readUnreadable(IInStream* stream, int c) {
    (void)stream;
    (void)c;
    throw SxReaderError("unreadable form");
}

// #'foo => (var foo)
static Obj* readVar(IInStream* stream, int c) {
    (void)c;
    Obj* x = read(stream, true);
    return List::create(x)->cons(rt::SYM_VAR);
}

static Obj* readMeta(IInStream* stream, int c) {
    (void)c;
    Obj* meta = read(stream, true);
    // std::cout << "readMeta:" << rt::toString(meta) << std::endl;
    if (pSymbol(meta) || pKeyword(meta) || pString(meta))
        meta = rt::assoc(Hashmap::create(), rt::KW_TAG, meta);
    else if (!pHashmap(meta))
        throw SxReaderError("#^ meta data must be a string, symbol, keyword,"
                            " or hashmap, got: " + rt::typeName(meta));
    return rt::withMeta(read(stream, true, NIL), pHashmap(meta));
}

// =========================================================================
// fn shorthand reader

typedef std::map<int,           // argument index [1 2 3 ... N]
                 Symbol*,
                 std::less<int>,
                 gc_allocator<std::pair<int const,
                                        Symbol*>>> argmap_t;

static argmap_t argMap;
static bool inReadFn = false;

/*
  %  => p1__NNN#
  %& => rest__NNN#
  %N => pN__NNN#
*/
static Symbol* gArg(int n) {
    std::stringstream ss;
    if (n == -1)
        ss << "rest";
    else
        ss << "p" << n;
    ss << "__" << rt::nextID() << "#";
    return Symbol::create(ss.str());
}

static Symbol* registerArg(int n) {
    if (!inReadFn)
        throw SxReaderError("arg literal not in #(...)");
    Symbol* ret = pSymbol(argMap[n]);
    if (!ret) {
        ret = gArg(n);
        argMap[n] = ret;
    }
    return ret;
}

// % %N %&
static Obj* readArg(IInStream* stream, int c) {
    if (!inReadFn)
        return interpretToken(readToken(stream, c));
    int ch = stream->peek();
    if (stream->eof() || isWhitespace(ch) || isTerminatingMacroChar(ch))
        return registerArg(1);  // %
    Obj* n = read(stream, true, NIL);
    if (rt::isEqualTo(n, rt::SYM_AMP))
        return registerArg(-1); // %&
    else if (Integer* p = pInteger(n))
        return registerArg(p->val());
    throw SxReaderError("arg literals must be % %N or %&, got: %"
                        + rt::toString(n));
}

// #(...)
static Obj* readFn(IInStream* stream, int c) {
    (void)c;
    if (inReadFn)
        throw SxReaderError("nested #(...) are not permitted");
    inReadFn = true;
    argMap.clear();
    stream->unget();            // push the ( back
    Obj* form = read(stream, true, NIL);
    Vector* fnArgs = Vector::create();
    auto itr = argMap.rbegin();
    /*
      If the form request args %2 and %5, the resulting arg vector must fill
      in the missing args:
      
      #(cons %2 %5) => (fn [%p2 %p3 %p4 %p5] (cons %p2 %p5))
    */
    if (itr != argMap.rend()) {
        int maxArg = itr->first;
        if (maxArg > 0)
            for (int i=1; i<=maxArg; ++i) {
                auto itr2 = argMap.find(i);
                if (itr2 == argMap.end())
                    fnArgs->conj(gArg(i));
                else
                    fnArgs->conj(itr2->second);
            }
    }
    auto itr3 = argMap.find(-1);
    if (itr3 != argMap.end())
        fnArgs->conj(rt::SYM_AMP)->conj(itr3->second);
    inReadFn = false;
    return List::create(rt::SYM_FN, fnArgs, form);
}

// =========================================================================
// read

static Obj* read(IInStream* stream, bool eofIsError, Obj* eofObject) {
    while (true) {
        int ch;
        ch = stream->get();
        while (isWhitespace(ch))
            ch = stream->get();
        if (stream->eof()) {
            if (eofIsError)
                throw SxReaderError("EOF while reading");
            return eofObject;
        }
        if (isdigit(ch))
            return readNumber(stream, ch);
        auto itr = macroMap.find(ch);
        if (itr != macroMap.end()) {
            Obj* x = macroMap[ch](stream, ch);
            if (x == stream)
                continue;
            return x;
        }
        if ('+' == ch || '-' == ch) {
            int ch2 = stream->get();
            if (isdigit(ch2) || '.' == ch2) {
                stream->unget();
                return readNumber(stream, ch);
            }
            stream->unget();    // +foo+ +++ (symbol, not a number)
        }
        else if ('.' == ch) {
            int ch2 = stream->get();
            if (isdigit(ch2)) {
                stream->unget();
                return readNumber(stream, ch);
            }
            stream->unget(); // .foo .== (symbol, not a number)
        }
        std::string token = readToken(stream, ch);
        return interpretToken(token);
    }
}

Obj* readOne(IInStream* stream, bool eofIsError, Obj* eofObject) {
    // argMap.clear();
    inReadFn = false;
    return read(stream, eofIsError, eofObject);
}

// =========================================================================
// quasi-quote

// is form (unquote ...) ?
static bool isUnquote(Obj* form) {
    return pList(form) && pList(form)->first() == rt::SYM_UNQUOTE;
}

// is form (unquote-splicing ...) ?
static bool isUnquoteSplicing(Obj* form) {
    return pList(form) && pList(form)->first() == rt::SYM_UNQUOTE_SPLICING;
}

// {a b, c d} => [a b c d]
// {}         => []
static Vector* flattenMap(Hashmap* m) {
    Vector* v = Vector::create();
    for (const auto& e : m->impl())
        v->conj(e.first)->conj(e.second);
    return v;
}

static Obj* syntaxQuote(Obj*, Hashmap*);

// (~e1 ~@e2 ... eN) => ((list e1) e2 ... (list eN))
// ()                => nil
// nil               => nil
static ISeq* seqExpandList(ISeq* s, Hashmap* m) {
    Vector* ret = Vector::create();
    for (; s; s=s->next()) {
        Obj* item = s->first();
        if (isUnquote(item))
            // ~e => (list e)
            ret->conj(List::create(rt::SYM_LIST, rt::second(item)));
        else if (isUnquoteSplicing(item))
            // ~@e => e
            ret->conj(rt::second(item));
        else
            ret->conj(List::create(rt::SYM_LIST, syntaxQuote(item, m)));
    }
    return ret->seq();
}

static Obj* syntaxQuote(Obj* obj, Hashmap* m) {
    Obj* ret = nullptr;
    if (rt::isSpecial(obj))
        // let => (quote let)
        ret = List::create(rt::SYM_QUOTE, obj);
    else if (Symbol* sym = pSymbol(obj)) {
        // bar#    => (quote bar__999__AUTO__)
        if (!sym->hasNS() && sym->name().back() == '#') {
            Symbol* gs = nullptr;
            if (!(gs = pSymbol(m->valAt(sym)))) {
                std::string name =
                    rt::genName(sym->name().substr(0, sym->name().size() - 1)
                                + "__",
                                "__AUTO__");
                m->assoc(sym, (gs = Symbol::create(name)));
            }
            sym = gs;
        }
        // TODO: this all needs to be refactored
        else if (!sym->hasNS()
                 && rt::currentNS()->get(sym)
                 && !pVar(rt::currentNS()->get(sym)))
            ;       // do nothing, it's an interned constant, don't qualify it
        else
            // foo/bar => foo/bar
            // bar     => namespace-name/bar
            try {
                sym = compiler::resolveSymbol(sym);
            }
            catch (SxCompilerError& e) {
                throw SxReaderError(e.what());
            }
        ret = List::create(rt::SYM_QUOTE, sym);
    }
    else if (isUnquote(obj))
        // (unquote x) => x
        ret = rt::second(obj);
    else if (isUnquoteSplicing(obj))
        throw SxReaderError("quasi-quote splice not in list");
    else if (List* lst = pList(obj)) {
        if (lst->isEmpty())
            // () => (list)
            ret = List::create(rt::SYM_LIST);
        else
            // (seq (concat ...))
            ret = List::create(rt::SYM_SEQ,
                               rt::cons(seqExpandList(lst, m),
                                        rt::SYM_CONCAT));
    }
    else if (Vector* v = pVector(obj))
        // (apply vector (seq (concat ...))
        ret = List::create(rt::SYM_APPLY,
                           rt::SYM_VECTOR,
                           List::create(rt::SYM_SEQ,
                                        rt::cons(seqExpandList(v->seq(), m),
                                                 rt::SYM_CONCAT)));
    else if (Hashmap* hm = pHashmap(obj)) {
        Vector* kvs = flattenMap(pHashmap(hm));
        // (apply hashmap (seq (concat ...)))
        ret = List::create(rt::SYM_APPLY,
                           rt::SYM_HASHMAP,
                           List::create(rt::SYM_SEQ,
                                        rt::cons(seqExpandList(kvs->seq(), m),
                                                 rt::SYM_CONCAT)));
    }
    else if (Hashset* hs = pHashset(obj)) {
        // (apply hashset (seq (concat ...)))
        ret = List::create(rt::SYM_APPLY,
                           rt::SYM_HASHSET,
                           List::create(rt::SYM_SEQ,
                                        rt::cons(seqExpandList(hs->seq(), m),
                                                 rt::SYM_CONCAT)));
    }
    else if (pINumber(obj)
             || pString(obj)
             || pKeyword(obj)
             || pCharacter(obj))
        ret = obj;
    else
        // x => (quote x)
        ret = List::create(rt::SYM_QUOTE, obj);
    return ret;
}

// `form => result
static Obj* readSyntaxQuote(IInStream* stream, int c) {
    (void)c;
    Hashmap* gensyms = Hashmap::create();
    Obj* form = read(stream, true);
    Obj* ret = syntaxQuote(form, gensyms);
    gensyms = nullptr;                   // gc
    return ret;
}

// ~form  => (unquote form)
// ~@form => (unquote-splicing form)
static Obj* readUnquote(IInStream* stream, int c) {
    c = stream->get();
    if (stream->eof())
        throw SxReaderError("end of input while reading unquote");
    if ('@' == c) {
        Obj* x = read(stream, true);
        return List::create(x)->cons(rt::SYM_UNQUOTE_SPLICING);
    }
    stream->unget();
    Obj* x = read(stream, true);
    return List::create(x)->cons(rt::SYM_UNQUOTE);
}

// #/pat/im
static Obj* readRegex(IInStream* stream, int ch) {
    std::string pat;
    for (ch=stream->get(); ch!='/'; ch=stream->get()) {
        if (stream->eof())
            throw SxReaderError("EOF while reading literal regex.");
        if (ch == '\\') {
            int ch2 = stream->get();
            if (stream->eof())
                throw SxReaderError("EOF while reading literal regex.");
            if (ch2 == '/') {
                pat += ch2;
                continue;
            }
            stream->unget();
        }
        pat += ch;
    }
    /*
    std::regex::flag_type flags = std::regex_constants::ECMAScript;
    ch = stream->get();
    if (std::isalpha(ch)) {
        if (std::strchr("imsx", ch)) {
            std::string opts = readToken(stream, ch);
            for (auto c : opts) {
                switch (c) {
                    case 'i': flags |= std::regex_constants::icase; break;
                    case 'm': flags |= std::regex_constants::multiline; break;
                    default: {
                        std::stringstream ss;
                        ss << "illegal literal regex option: " << c
                           << "must be one of: im (case sensitive)";
                        throw SxReaderError(ss.str());
                    }
                }
            }
        }
        else {
            std::stringstream ss;
            ss << "illegal literal regex option: " << ch
               << ;
            throw SxReaderError(ss.str());
        }
    }
    else
        stream->unget();
    */
    return Regex::create(pat);
}

// =========================================================================
// dispatch maps

static mapChFn_t initMacroMap() {
    mapChFn_t m;
    m[';'] = readComment;
    m['\''] = readQuote;
    m['('] = readList;
    m[')'] = readUnmatchedDelimiter;
    m['['] = readVector;
    m[']'] = readUnmatchedDelimiter;
    m['{'] = readHashmap;
    m['}'] = readUnmatchedDelimiter;
    m['"'] = readString;
    m['\\'] = readCharacter;
    m['#'] = readDispatch;
    m['~'] = readUnquote;
    m['`'] = readSyntaxQuote;
    m['%'] = readArg;
    return m;
}

static mapChFn_t initDispatchMacroMap() {
    mapChFn_t m;
    m['_'] = readDiscard;
    m['<'] = readUnreadable;
    m['\''] = readVar;
    m['^'] = readMeta;
    m['{'] = readHashset;
    m['('] = readFn;
    m['/'] = readRegex;
    return m;
}

} // end namespace reader
