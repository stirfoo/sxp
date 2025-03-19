/*
  cfn.hpp
  S. Edward Dolan
  Wednesday, May 24 2023
*/

#ifndef CFN_HPP_INCLUDED
#define CFN_HPP_INCLUDED

typedef Obj*(*cfn_t)(Obj**);
//typedef std::function<Obj*(Obj**)> cfnlambda_t;

struct CFn : Fn {
    static void initCFns();
    static CFn* create(const std::string& name, cfn_t cfn);
    //static CFn* create(const std::string& name, cfnlambda_t lambda);
    std::string typeName();
    std::string toString();
    FnMethod* addMethod(int reqArgs);
    cfn_t cfn() { return _cfn; }
    //cfnlambda_t lambda() { return _lambda; }
protected:
    CFn(const std::string& name, cfn_t cfn);
    cfn_t _cfn;
    //cfnlambda_t _lambda;
};
DEF_CASTER(CFn)

#endif // PROC_HPP_INCLUDED
