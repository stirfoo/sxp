/*
  vm.hpp
  S. Edward Dolan
  Saturday, May 13 2023
*/

#ifndef VM_HPP_INCLUDED
#define VM_HPP_INCLUDED


struct Frame;

struct VM {
    VM();
    ~VM();
    Obj* run(Obj*);
    static long nInstructions() { return _nInstructions; }
protected:
    static constexpr int MAX_PSTACK_SIZE = 512000;
    int pc;                     // program counter
    vecobj_t pstack;            // parameter stack
    Frame* curFrame = nullptr;  // currently executing method frame
    std::vector<Frame*, gc_allocator<Frame*>> fstack; // frame stack
    Upval* openUpvals;                                // ???
    std::vector<uint16_t> jstack;
    static long _nInstructions;
    void jpush(uint16_t addr) {
        std::cout << "JPUSH: " << HEX4(addr) << std::endl;
        jstack.push_back(addr);
    }
    uint16_t jpop() {
        uint16_t addr = jstack.back();
        std::cout << "JPOP: " << HEX4(addr) << std::endl;
        jstack.pop_back();
        return addr;
    }
    void reset();
    void ppush(Obj*);
    Obj* ppop();
    Obj* ppeek(int i=0);
    void fpush(FnMethod*, int, int, Closure*);
    int fpop(bool isThrow = false);
    void doCall(Obj*, int);
    void printTrace();
    Upval* captureUpval(uint8_t index);
    void closeUpvals(Obj** lastAddr);
    void printStack(std::ostream&);
};

#endif // VM_HPP_INCLUDED
