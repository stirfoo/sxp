/*
  stream.cpp
  S. Edward Dolan
  Wednesday, May 10 2023
*/

#include "sxp.hpp"

openMode_t kwToOpenMode(Keyword* kw) {
    if (kw == rt::KW_APP) return std::ios_base::app;
    else if (kw == rt::KW_BINARY) return std::ios_base::binary;
    else if (kw == rt::KW_IN) return std::ios_base::in;
    else if (kw == rt::KW_OUT) return std::ios_base::out;
    else if (kw == rt::KW_TRUNC) return std::ios_base::trunc;
    else if (kw == rt::KW_ATE) return std::ios_base::ate;
    else
        throw SxIOError("unrecognized stream mode: " + kw->toString());
}

SysInStream* SysInStream::create(const std::string& name, std::istream& s) {
    if (rt::SYS_IN)
        return rt::SYS_IN;
    return new SysInStream(name, s);
}

SysOutStream* SysOutStream::create(const std::string& name, std::ostream& s) {
    if (name == "stdout") {
        if (rt::SYS_OUT)
            return rt::SYS_OUT;
        return new SysOutStream(name, s);
    }
    else if (name == "stderr") {
        if (rt::SYS_ERR)
            return rt::SYS_ERR;
        return new SysOutStream(name, s);
    }
    else
        throw SxError("Attemt to create an unknown SysOutStream."
                      "sxp provides stdout and stderr");
    
}

void SysInStream::close() {
    throw SxIOError("can't close system stream: " + _name);
}

void SysOutStream::close() {
    throw SxIOError("can't close system stream: " + _name);
}

// =========================================================================
// Fstream

void FStream::close() {
    if (!_s->is_open())
        return;
    _s->close();
}

void FStream::print(Obj* x) { 
    ensureOpen();
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << (!x ? "nil" : x->toString()); 
}

void FStream::println(Obj* x) { 
    ensureOpen();
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError(_name + " is not an output stream"); 
    (*_s) << (!x ? "nil" : x->toString()) << '\n'; 
}
 
void FStream::print(const std::string& s) { 
    ensureOpen();
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << s; 
}
 
void FStream::println(const std::string& s) { 
    ensureOpen();
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError(_name + " is not an output stream"); 
    (*_s) << s << '\n'; 
}

// =========================================================================
// SStream

void SStream::close() {
    throw SxIOError("can't close a string stream");
}

void SStream::print(Obj* x) { 
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << (!x ? "nil" : x->toString()); 
}

void SStream::println(Obj* x) { 
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << (!x ? "nil" : x->toString()) << '\n'; 
}
 
void SStream::print(const std::string& s) { 
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << s; 
}
 
void SStream::println(const std::string& s) { 
    if (!(_mode & std::ios_base::out)) 
        throw SxIOError("SStream is not an output stream"); 
    (*_s) << s << '\n'; 
}

void finalizer(void* obj, void* dat) {
    (void)dat;
    if (static_cast<FStream*>(obj))
        static_cast<FStream*>(obj)->finalize();
    else
        static_cast<SStream*>(obj)->finalize();
}
