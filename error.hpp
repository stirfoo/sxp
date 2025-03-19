/*
  error.hpp
  S. Edward Dolan
  Monday, May  8 2023
*/

#ifndef ERROR_HPP_INCLUDED
#define ERROR_HPP_INCLUDED

struct SxCastError;

struct SxError : Obj, std::runtime_error {
    SxError()
        : std::runtime_error("an unknown error occurred") {
        _typeName = "SxError";
    }
    SxError(const std::string& msg)
        : std::runtime_error(msg) {
        _typeName = "SxError";
    }
    virtual ~SxError() throw() {}
    virtual SxError* clone(std::string msg) {
        return new (PointerFreeGC) SxError(msg);
    }
    std::string toString() {
        size_t maxMsgLen = 30;
        std::stringstream ss;
        ss << "#<" << _typeName << " \"";
        std::string w(what());
        size_t n = w.size();
        if (n > maxMsgLen)
            ss << w.substr(0, std::min<int>(n, maxMsgLen)) << "...\" ";
        else
            ss << w << "\" ";
        ss << this << '>';
        return ss.str();
    }
};

struct SxCastError : SxError {
    SxCastError() : SxError() { _typeName = "SxCastError"; }
    SxCastError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxCastError";
    }
    SxCastError(const std::string& fromName, const std::string& toName)
        : SxError("cannot cast: " + fromName + " -> " + toName) {
        _typeName = "SxCastError";
    }
    SxCastError* clone(std::string msg) {
        return new (PointerFreeGC) SxCastError(msg);
    }
};
DEF_CASTER(SxCastError)

DEF_CASTER(SxError)             // NOTE: must be after SxCastError

struct SxAny: SxError {
    SxAny() : SxError() {_typeName = "SxAny";}
    SxAny* clone() {
        return new (PointerFreeGC) SxAny();
    }
};
DEF_CASTER(SxAny)

struct SxIOError : SxError {
    SxIOError() : SxError() {_typeName = "SxIOError";}
    SxIOError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxIOError";
    }
    SxIOError* clone(std::string msg) {
        return new (PointerFreeGC) SxIOError(msg);
    }
};
DEF_CASTER(SxIOError)

struct SxReaderError : SxError {
    SxReaderError() : SxError() {_typeName = "SxReaderError";}
    SxReaderError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxReaderError";
    }
    SxReaderError* clone(std::string msg) {
        return new (PointerFreeGC) SxReaderError(msg);
    }
};
DEF_CASTER(SxReaderError)

struct SxRuntimeError : SxError {
    SxRuntimeError()
        : SxError() {
        _typeName = "SxRuntimeError";
    }
    SxRuntimeError(const std::string& msg)
        : SxError(msg){
        _typeName = "SxRuntimeError";
    }
    SxRuntimeError* clone(std::string msg) {
        return new (PointerFreeGC) SxRuntimeError(msg);
    }
};
DEF_CASTER(SxRuntimeError)

struct SxCompilerError : SxError {
    SxCompilerError() : SxError() { _typeName = "SxCompilerError"; }
    SxCompilerError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxCompilerError";
    }
    SxCompilerError* clone(std::string msg) {
        return new (PointerFreeGC) SxCompilerError(msg);
    }
};
DEF_CASTER(SxCompilerError)

struct SxSyntaxError : SxError {
    SxSyntaxError() : SxError() { _typeName = "SxSyntaxError"; }
    SxSyntaxError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxSyntaxError";
    }
    SxSyntaxError* clone(std::string msg) {
        return new (PointerFreeGC) SxSyntaxError(msg);
    }
};
DEF_CASTER(SxSyntaxError)

struct SxArithmeticError : SxError {
    SxArithmeticError() : SxError() { _typeName = "SxArithmeticError"; }
    SxArithmeticError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxArithmeticError";
    }
    SxArithmeticError* clone(std::string msg) {
        return new (PointerFreeGC) SxArithmeticError(msg);
    }
};
DEF_CASTER(SxArithmeticError)

struct SxOutOfBoundsError : SxError {
    SxOutOfBoundsError() : SxError() { _typeName = "SxOutOfBoundsError"; }
    SxOutOfBoundsError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxOutOfBoundsError";
    }
    SxOutOfBoundsError* clone(std::string msg) {
        return new (PointerFreeGC) SxOutOfBoundsError(msg);
    }
};
DEF_CASTER(SxOutOfBoundsError)

struct SxNotImplementedError : SxError {
    SxNotImplementedError() : SxError() {
        _typeName = "SxNotImplementedError";
    }
    SxNotImplementedError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxNotImplementedError";
    }
    SxNotImplementedError* clone(std::string msg) {
        return new (PointerFreeGC) SxNotImplementedError(msg);
    }
};
DEF_CASTER(SxNotImplementedError)

struct SxIllegalArgumentError : SxError {
    SxIllegalArgumentError()
        : SxError() {
        _typeName = "SxIllegalArgumentError";
    }
    SxIllegalArgumentError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxIllegalArgumentError";
    }
    SxIllegalArgumentError* clone(std::string msg) {
        return new (PointerFreeGC) SxIllegalArgumentError(msg);
    }
};
DEF_CASTER(SxIllegalArgumentError)

struct SxSortError : SxError {
    SxSortError() : SxError() { _typeName = "SxSortError"; }
    SxSortError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxSortError";
    }
    SxSortError(const std::string& t1, const std::string& t2)
        : SxError(t1 + " and " + t2 + " cannot be lexicographically sorted") {
        _typeName = "SxSortError";
    }
    SxSortError* clone(std::string msg) {
        return new (PointerFreeGC) SxSortError(msg);
    }
};
DEF_CASTER(SxSortError)

struct SxRegexError : SxError {
    SxRegexError() : SxError() {
        _typeName = "SxRegexError";
    }
    SxRegexError(const std::string& msg)
        : SxError(msg) {
        _typeName = "SxRegexError";
    }
    SxRegexError* clone(std::string msg) {
        return new (PointerFreeGC) SxRegexError(msg);
    }
};
DEF_CASTER(SxRegexError)


#endif // ERROR_HPP_INCLUDED
