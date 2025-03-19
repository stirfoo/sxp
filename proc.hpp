/*
  proc.hpp
  S. Edward Dolan
  Wednesday, May 24 2023
*/

#ifndef PROC_HPP_INCLUDED
#define PROC_HPP_INCLUDED

/*
  A sxp procedure. A Proc is a Fn whoes bytecode contains 2 instructions:

  CALL_PROC
  RETURN
  
  CALL_PROC is followed by a hard-coded 2-byte operand that points to a case
  label in the VM's main switch statement. This bytecode can be viewed with:

  (fn-dump proc-name).

  See the Proc::addMethod() implementation.
 */
struct Proc : Fn {
    static Proc* create(const std::string& name);
    std::string toString();
    FnMethod* addMethod(bool isRest, int reqArgs, uint16_t id);
    static void initProcs();
protected:
    Proc(const std::string& name);
};
DEF_CASTER(Proc)

#endif // PROC_HPP_INCLUDED
