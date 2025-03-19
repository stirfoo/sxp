/*
  fn.hpp
  S. Edward Dolan
  Sunday, May  7 2023
*/

#ifndef FN_HPP_INCLUDED
#define FN_HPP_INCLUDED

struct Fn;

/*
  (try
    <...
    protected code here
    ...>
    (catch SxIOError e
      ...)
    (catch SxArithmeticError e
      ...)
    (catch SxError e
      ...)
    (finally
      ...))
      
   (throw (error SxError "shizzle!"))
 */

struct ThrowHandler : Obj {
    friend struct FnMethod;
    static ThrowHandler* create(int psa, int pea, int hsa, SxError* e) {
        return new ThrowHandler(psa, pea, hsa, e);        
    }
    std::string toString();
protected:
    int _startAddr;             // protected code start address (inclusive)
    /*
      _endAddr is the addres of the instruction after the protected code
      block.  But, the test needs to be <= in getHandlerAddr() because the VM
      sets the next opcode to be executed with a post increment on the program
      counter: oc = pc++.
    */
    int _endAddr;
    int _handlerAddr;           // CATCH block start addr
    SxError* _e;                // instance of type of error handled
    ThrowHandler(int psa, int pea, int hsa, SxError* e)
        : _startAddr(psa),
          _endAddr(pea),
          _handlerAddr(hsa),
          _e(e) {
        _typeName = "SxThrowHandler";
    }
};

struct FnMethod : Obj {
    friend struct Fn;
    const vecu8_t& bc() const { return _bytecode; }
    bool isRest() { return _isRest; }
    int reqArgs() { return _reqArgs; }
    int nLocals() const { return _nLocals; }
    void nLocals(int n) { _nLocals = n; } // for Proc
    int nextLocalIdx() { return _nLocals++; }
    const Fn* fn() const { return _fn; }
    void appendByte(uint8_t byte) { _bytecode.push_back(byte); }
    void rewrite(size_t addr);  // why size_t?
    uint16_t nextAddress() { return static_cast<uint16_t>(_bytecode.size()); };
    void dump(std::ostream&) const;
    int getHandlerAddr(uint16_t pc, const SxError& e) const;
    void addHandler(int psa, int pea, int hsa, SxError* e);
protected:
    vecu8_t _bytecode;
    bool _isRest;               // true if method is a rest method
    int _reqArgs;               // minimum required arguments (may be 0)
    int _nLocals;               // number of required local slots on the stack
    struct Fn* _fn;             // parent function
    std::vector<ThrowHandler*, gc_allocator<ThrowHandler*>> _handlers;
    FnMethod();
    FnMethod(bool isRest, int reqArgs, Fn* fn);
};
DEF_CASTER(FnMethod)

// =========================================================================

typedef std::map<int, FnMethod*, std::less<int>,
                 gc_allocator<std::pair<int const, FnMethod*>>> methodmap_t;

struct Fn : virtual Obj {
    static Fn* create(const std::string& name);
    std::string toString();
    size_t getHash();
    std::string name() const { return _name; }
    const vecobj_t& cp() const { return _cpool; }
    // 
    FnMethod* getMethod(int nArgs);
    FnMethod* addMethod(bool isRest, int reqArgs);
    int appendConstant(Obj* x);
    int nUpvals() { return _nUpvals; }
    void nUpvals(int n) { _nUpvals = n; }
    void dump(bool dumpMethods=true, std::ostream& = std::cout) const;
protected:
    size_t _hash;
    std::string _name;
    vecobj_t _cpool;            // constant pool
    FnMethod* _restMethod;      // may be nullptr
    methodmap_t _methods;       // map required arity (fixed) to FnMethod*
    int _nUpvals;               // number of closed over vars
    Fn(const std::string& name);
};
DEF_CASTER(Fn)

// =========================================================================

struct Upval : gc {
    static Upval* create(Obj** addr) { return new Upval(addr); }
    Obj** addr;
    Obj* val;
    Upval* next;
protected:
    Upval(Obj** addr) : addr(addr), val(nullptr), next(nullptr) {}
};
DEF_CASTER(Upval)

// =========================================================================

typedef std::vector<Upval*, gc_allocator<Upval*>> vecupval_t;

struct Closure : Obj {
    static Closure* create(Fn* fn);
    Fn* fn;
    vecupval_t upvals;
    std::string toString();
protected:
    Closure(Fn* fn) : fn(fn), upvals(fn->nUpvals()) {
        _typeName = "SxClosure";
    }
};
DEF_CASTER(Closure)

#endif // FN_HPP_INCLUDED
