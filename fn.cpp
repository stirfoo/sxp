/*
  fn.cpp
  S. Edward Dolan
  Sunday, May  7 2023
*/

#include "sxp.hpp"

std::string ThrowHandler::toString() {
    std::stringstream ss;
    ss << "psa=" << std::setw(4) << std::hex << std::setfill('0')
       << _startAddr << ' '
       << "pea=" << std::setw(4) << std::hex << std::setfill('0')
       << _endAddr << ' '
       << "hsa=" << std::setw(4) << std::hex << std::setfill('0')
       << _handlerAddr << ' '
       << _e->typeName();
    return ss.str();
}

FnMethod::FnMethod() : _bytecode(),
                       _isRest(false),
                       _reqArgs(0),
                       _nLocals(0),
                       _fn(nullptr),
                       _handlers() {
    _typeName = "SxFnMethod";
}

FnMethod::FnMethod(bool isRest, int reqArgs, Fn* fn)
    : _bytecode(),
      _isRest(isRest),
      _reqArgs(reqArgs),
      _nLocals(0),
      _fn(fn),
      _handlers() {
    _typeName = "SxFnMethod";
}

void FnMethod::rewrite(size_t addr) {
    assert(addr < _bytecode.size() - 1);
    int i = _bytecode.size();
    _bytecode[addr] = i & 0xff;
    _bytecode[addr + 1] = i >> 8;
}

void FnMethod::dump(std::ostream& s) const {
    s << "----------------------------------------------------------------"
        "FnMethod-------\n"
      << "    rest: " << (_isRest ? "true" : "false") << '\n'
      << " reqArgs: " << _reqArgs << '\n'
      << " nLocals: " << std::dec << _nLocals << '\n'
      << "      bc: ";
    size_t i = 0;
    while (i != _bytecode.size()) {
        s << (i ? "          " : "");
        i = vasm::disOne(this, i, s);
    }
    s << "handlers: ";
    i = 0;
    while (i != _handlers.size()) {
        s << (i ? "          " : "") << rt::toString(_handlers[i]) << '\n';
        ++i;
    }
    if (_handlers.empty())
        s << '\n';
    s << "-----------------------------------------------------------------"
        "--------------" << std::endl;
}

int FnMethod::getHandlerAddr(uint16_t pc, const SxError& err) const {
    // std::cout << _fn->name() << ": getHandlerAddr: pc:" << pc << " n:"
    //           << _handlers.size() << std::endl;
    for (auto h : _handlers) {
        // std::cout << "ename:" << h->_e->typeName()
        //           << " ename:" << err.typeName()
        //           << " hsa: " << h->_startAddr << " hea:" << h->_endAddr
        //           << std::endl;
        if ((err.typeName() == h->_e->typeName()
             || h->_e->typeName() == "SxError"
             || h->_e->typeName() == "SxAny")
            && pc >= h->_startAddr
            && pc <= h->_endAddr) {
            // puts("YES!");
            return h->_handlerAddr;
        }
    }
    // puts("NO!");
    return -1;                  // not found
}

void FnMethod::addHandler(int psa, int pea, int hsa, SxError* e) {
    _handlers.push_back(ThrowHandler::create(psa, pea, hsa, e));
}


// =========================================================================
// Fn

Fn* Fn::create(const std::string& name) {
    return new Fn(name);
}

Fn::Fn(const std::string& name)
    : _hash(std::hash<Fn*>()(this)),
      _name(name),
      _cpool(),
      _restMethod(nullptr),
      _methods(),
      _nUpvals(0) {
    _typeName = "SxFn";
}

std::string Fn::toString() {
    std::stringstream ss;
    ss << "#<" << _typeName << ' ' << _name << ' ' << this << '>';
    return ss.str();
}

size_t Fn::getHash() {
    return _hash;
}

FnMethod* Fn::getMethod(int nArgs) {
    auto itr = _methods.find(nArgs);
    if (itr != _methods.end())
        return itr->second;     // fixed arity method
    if (_restMethod && nArgs >= _restMethod->_reqArgs)
        return _restMethod;     // rest method
    return nullptr;             // not found
}

FnMethod* Fn::addMethod(bool isRest, int reqArgs) {
    // std::cout << "addMethod "  << isRest << ", " << reqArgs << std::endl;
    if (isRest) {
        if (_restMethod)
            throw SxCompilerError("FN cannot have multiple rest methods");
        return _restMethod = new FnMethod(true, reqArgs, this);
    }
    else if (_methods.find(reqArgs) != _methods.end()) {
        std::stringstream ss;
        ss << "ambiguous FN method with " << reqArgs << " parameters";
        throw SxCompilerError(ss.str());
    }
    else
        return _methods[reqArgs] = new FnMethod(false, reqArgs, this);
}

int Fn::appendConstant(Obj* x) {
    for (size_t i=0; i<_cpool.size(); ++i) {
        INumber* y = pINumber(x);
        if (y) {
            // Compare numbers by value so 3.3, 3/4, and 1024 will each have
            // exactly one slot in the fn's constant pool. Some smaller
            // integers are cached, so they all can't be compared by pointer.
            if (y->isEqualTo(_cpool[i]))
                return i;
        }
        else
            // Compare everything else by identity, strings, keywords and
            // characters are cached, so :foo is always :foo, "foo" is "foo",
            // and \space is \space. However, '(1 2 3) may appear multiple
            // times in the cpool because it is mutable:
            // (let [x '(1 2 3) y '(1 2 3)] (set-nth! 1 y :two))
            if (y == x)
                return i;
    }
    _cpool.push_back(x);
    return _cpool.size() - 1;
}

void Fn::dump(bool dumpMethods, std::ostream& s) const {
    s << "================================================================Fn=="
        "===========\n"
      << "    name: " << _name << '\n'
      << "    hash: " << _hash << '\n'
      << " nUpvals: " << _nUpvals << '\n'
      << "      cp: ";
    for (size_t i=0; i<_cpool.size(); ++i) {
        if (i)
            s << "          [" << i << "] ";
        else
            s << "[" << i << "] ";
        s << rt::toString(_cpool[i]) << '\n';
    }
    if (_cpool.size() == 0)
        s << std::endl;
    if (dumpMethods) {
        for (auto m : _methods)
            m.second->dump(s);
        if (_restMethod)
            _restMethod->dump(s);
    }
}

// =========================================================================
// Upval

// =========================================================================
// Closure

Closure* Closure::create(Fn* fn) {
    return new Closure(fn);
}

std::string Closure::toString() {
    std::stringstream ss;
    ss << "#<" << typeName() << ' ' << fn->name() << ' ' << this << '>';
    return ss.str();
}

