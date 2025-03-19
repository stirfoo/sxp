/*
  vm.cpp
  S. Edward Dolan
  Thursday, June  8 2023
*/

#include "sxp.hpp"

struct Frame : gc {
    const Fn* fn;            // method's parent function
    const FnMethod* method;  // method currently being executed
    const vecu8_t& bc;       // method's bytecode
    const vecobj_t& cp;      // fn's constant pool
    Obj** locals;            // pstack address of fn
    uint16_t retAddr;        // the addr of the next instruction of the caller
    Closure* closure;
    int fnIndex;                // pstack index of this frame's function
    Frame(const FnMethod* method, Obj** locals, uint16_t retAddr,
          Closure* closure, int fnIndex)
        : fn(method->fn()),
          method(method),
          bc(method->bc()),
          cp(method->fn()->cp()),
          locals(locals),
          retAddr(retAddr),
          closure(closure),
          fnIndex(fnIndex) { 
    }
};

// =========================================================================

long VM::_nInstructions = 0;

VM::VM()
    : pc(0),
      pstack(),
      curFrame(nullptr),
      fstack(),
      openUpvals(nullptr),
      jstack() {
    pstack.reserve(MAX_PSTACK_SIZE);
    rt::pushVM(this);
}

VM::~VM() {
    rt::popVM();
}

void VM::reset() {
    pc = 0;
    pstack.clear();
    curFrame = nullptr;
    openUpvals = nullptr;
    fstack.clear();
}

void VM::ppush(Obj* x) {
    pstack.push_back(x);
}

Obj* VM::ppop() {
    Obj* x = pstack.back();
    pstack.pop_back();
    return x;
}

// i=0 is the top of the stack, i=1 is next obj down, etc.
Obj* VM::ppeek(int i) {
    return pstack[pstack.size() - 1 - i];
}

/*
  nArgs is the number of args on the stack after accumulating any & rest args
  into a list.
 */
void VM::fpush(FnMethod* method, int nArgs, int retAddr,
               Closure* closure = nullptr ) {
    int fnIndex = pstack.size() - nArgs - 1; // stack index of called fn
    Obj** locals = &pstack[fnIndex]; // ???
    // add nil's for the method's locals (the fn is at locals[0])
    size_t newSize = pstack.size() + method->nLocals() - nArgs - 1;
    if (newSize > MAX_PSTACK_SIZE) {
        std::stringstream ss;
        ss << "max VM parameter stack size (" << MAX_PSTACK_SIZE
           << ") exceeded";
        throw SxRuntimeError(ss.str());
    }
    pstack.resize(newSize);
    fstack.push_back(curFrame = new Frame(method, locals, retAddr, closure,
                                          fnIndex));
    pc = 0;
}

int VM::fpop(bool isThrow) {
    if (!isThrow)
        if (openUpvals)
            closeUpvals(curFrame->locals);
    Frame* f = fstack.back();   // get a ref to this frame
    fstack.pop_back();          // pop this frame
    if (fstack.empty())
        /*
          The last frame was popped, leave the result of this fn call on the
          stack.
        */
        return true; 
    pc = f->retAddr;            // reset the pc for the caller of this fn
    Obj* x = pstack.back();     // grab the result from this fn call
    pstack.resize(f->fnIndex);  // pop this fn and everything after it
    ppush(x);                   // push the result for the caller
    curFrame = fstack.back();   // reset the current frame
    return false;
}

/*
  nArgs here is how many arguments the fn was called with. This differs from
  the nArgs parameter in VM::fpush().
*/
void VM::doCall(Obj* callable, int nArgs) {
    Fn* fn = (pClosure(callable) ?
              pClosure(callable)->fn :
              cpFn(callable)); // may throw
    FnMethod* m = fn->getMethod(nArgs);
    if (!m) {
        std::stringstream ss;
        ss << "wrong number of args (" << nArgs << ") passed to: "
           << fn->name();
        throw SxRuntimeError(ss.str());
    }
    if (!m->isRest())
        fpush(m, nArgs, pc, pClosure(callable));
    else {
        // rest method with no tail args
        if (m->reqArgs() == nArgs) {
            ppush(NIL);
            fpush(m, nArgs + 1, pc, pClosure(callable));
        }
        else {
            // rest method with tail args
            int nTailArgs = nArgs - m->reqArgs();
            ISeq* tail = List::create(ppop());
            while (--nTailArgs)
                tail = rt::cons(tail, ppop());
            ppush(tail);
            fpush(m, m->reqArgs() + 1, pc, pClosure(callable));
        }
    }
}

// sxp function: (vm-stack)
void VM::printStack(std::ostream& stream=std::cout) {
    if (!pstack.empty()) {
        size_t i = 0, maxLen = 73; // stack object max toString() length
        stream << "> ";
        for (auto itr=pstack.rbegin(); itr!= pstack.rend(); ++itr) {
            if (i++)
                stream << "  ";
            const std::string& s = rt::toString(*itr);
            stream << (s.size() > maxLen ? s.substr(0, maxLen) + "..." : s)
                   << std::endl;
        }
    }
}

void VM::printTrace() {
    printStack();
    vasm::disOne(curFrame->method, pc, std::cout);
}

Upval* VM::captureUpval(uint8_t index) {
    Obj** localAddr = &curFrame->locals[index];
    Upval* prev = nullptr, *cuv = openUpvals;
    while (cuv && cuv->addr > localAddr) {
        prev = cuv;
        cuv = cuv->next;
    }
    if (cuv && cuv->addr == localAddr)
        return cuv;
    Upval* nuv = Upval::create(localAddr);
    nuv->next = cuv;
    if (!prev)
        openUpvals = nuv;
    else
        prev->next = nuv;
    return nuv;
}

void VM::closeUpvals(Obj** lastAddr) {
    while (openUpvals && openUpvals->addr >= lastAddr) {
        Upval* v = openUpvals;
        v->val = *v->addr;
        v->addr = &v->val;
        openUpvals = v->next;
        // std::cout << "CLOSING UPVAL: " << rt::toString(v->val)
        //           << std::endl;
    }
}
// read and return a 16 bit unsigned int at the current pc
#define READ_U16() (curFrame->bc[pc] | curFrame->bc[pc + 1] << 8)

// jump to a case label in VM::run()
#define GOTO(id) do {                           \
 oc = id;                                       \
 goto NEXTOC;                                   \
 } while(0)

/*
  Search the frame stack looking for an error handler. Used in the two catches
  at the end of VM::run(). The printTrace() is necessary because the GOTO skips
  the test at the top of the VM::vm() method. The `throw e' will occur if
  there are no methods left in this VM instance.
 */
#define HANDLE_ERROR()                                                  \
    int addr;                                                           \
    while ((addr = curFrame->method->getHandlerAddr(pc, *sxe)) < 0)     \
        if (fpop(true))                                                 \
            throw e;                                                    \
    pstack.resize(curFrame->fnIndex + curFrame->method->nLocals());     \
    pc = addr;                                                          \
    ppush(sxe);                                                         \
    if (rt::vmTrace)                                                    \
        printTrace();                                                   \
    GOTO(curFrame->method->bc()[pc++])

Obj* VM::run(Obj* fnOrClosure) {
    reset();
    ppush(fnOrClosure);
    doCall(fnOrClosure, 0);
    int oc;
    while (true) {
        if (rt::vmTrace)         // the order of these statements matters
            printTrace();        // ...
        oc = curFrame->bc[pc++]; // because of the p++ post increment
    NEXTOC:
        assert(pc <= 0xffff);
        ++_nInstructions;
        try {
            switch (oc) {
#include "instr_8.cpp"
#include "instr_16.cpp"
#include "proc_ns.cpp"
#include "proc_numbers.cpp"
#include "proc_io.cpp"
#include "proc_error.cpp"
#include "proc_predicate.cpp"
#include "proc_re.cpp"
                default: {
                    curFrame->fn->dump();
                    printStack();
                    std::cerr << "PC: " << HEX4(pc) << std::endl;
                    std::stringstream ss;
                    ss << "illegal instruction in VM: ";
                    if (oc > 255)
                        ss << HEX4(oc);
                    else
                        ss << HEX2(oc);
                    throw SxRuntimeError(ss.str());
                    break;
                }
            }
        }
        catch (SxError& e) {
            SxError* sxe = e.clone(e.what());
            HANDLE_ERROR();
        }
        catch (std::exception& e) {
            // wrap it in something sxp can fondle
            SxError* sxe = new (PointerFreeGC) SxError(e.what());
            HANDLE_ERROR();
        }
    }
}

#undef READ_U16
#undef GOTO
#undef HANDLE_ERROR
