/*
  stream.hpp
  S. Edward Dolan
  Tuesday, May  9 2023
*/

#ifndef STREAM_HPP_INCLUDED
#define STREAM_HPP_INCLUDED

// NOTE: openMode_t is typedef'd in xface.hpp

openMode_t kwToOpenMode(Keyword* kw);

inline std::string openModeToStr(openMode_t mode) {
    std::stringstream ss;
    ss << (mode & std::ios_base::app ? " :app" : "")
       << (mode & std::ios_base::binary ? " :binary" : "")
       << (mode & std::ios_base::in ? " :in" : "")
       << (mode & std::ios_base::out ? " :out" : "")
       << (mode & std::ios_base::trunc ? " :trunc" : "")
       << (mode & std::ios_base::ate ? " :ate" : "");
    return ss.str();
}

struct SysInStream : IInStream {
    static SysInStream* create(const std::string& name, std::istream& s);
    std::string toString() {
        std::stringstream ss;
        ss << "#<" << _typeName << ' ' << _name << openModeToStr(mode())
           << ' ' << this << '>';
        return ss.str();
    }
    openMode_t mode() { return std::ios_base::in; }
    bool eof() { return _s.eof(); }
    void close();
    int get() { return _s.get(); }
    int peek() { return _s.peek(); }
    void unget() { _s.unget(); }
    std::istream& istream() { return _s; }
protected:
    std::string _name;
    std::istream& _s;
    SysInStream(const std::string& name, std::istream& s)
        : _name(name), _s(s) {
        _typeName = "SxSysInStream";
    }
};
DEF_CASTER(SysInStream)

struct SysOutStream : IOutStream {
    static SysOutStream* create(const std::string& name, std::ostream& s);
    std::string toString() {
        std::stringstream ss;
        ss << "#<" << _typeName << ' ' << _name
           << openModeToStr(mode()) << ' ' << this << '>';
        return ss.str();
    }
    openMode_t mode() { return std::ios_base::out; }
    bool eof() { return _s.eof(); }
    void close();
    void put(int c) { _s.put(c); }
    void flush() { _s.flush(); }
    void print(Obj* x) { _s << (!x ? "nil" : x->toString()); }
    void println(Obj* x) { _s << (!x ? "nil" : x->toString()) << '\n'; }
    void print(const std::string& s) { _s << s; }
    void println(const std::string& s) { _s << s << '\n'; }
    std::ostream& ostream() { return _s; }
protected:
    std::string _name;
    std::ostream& _s;
    SysOutStream(const std::string& name, std::ostream& s)
        : _name(name), _s(s) {
        _typeName = "SxSysOutStream";
    }
};
DEF_CASTER(SysOutStream)

void finalizer(void*, void*);

struct FStream : IInStream, IOutStream {
    static FStream* create(const std::string& name,
                           openMode_t mode = (std::ios_base::in
                                              | std::ios_base::out)) {
        std::fstream* s = new std::fstream(name, mode);
        if (!s->is_open())
            throw SxIOError("failed to open: " + name);
        return ::new (GC, finalizer) FStream(name, mode, s);
    }
    std::string toString() {
        std::stringstream ss;
        ss << "#<" << typeName() << " \"" << _name << "\""
           << openModeToStr(_mode) << ' ' << this << '>';
        return ss.str();
    }
    std::string name() { return _name; }
    openMode_t mode() { return _mode; }
    bool eof() { ensureOpen(); return _s->eof(); }
    void close();
    void put(int c) {
        ensureOpen();
        if (!(_mode & std::ios_base::out))
            throw SxIOError("cant't write to input stream: " + _name);
        _s->put(c);
    }
    void flush() {
        ensureOpen();
        if (!(_mode & std::ios_base::out))
            throw SxIOError("can't flush input stream: " + _name);
        _s->flush();
    }
    int get() {
        ensureOpen();
        if (_mode & std::ios_base::out)
            throw SxIOError("can't read from output stream: " + _name);
        return _s->get();
    }
    int peek() {
        ensureOpen();
        if (_mode & std::ios_base::out)
            throw SxIOError("can't read from output stream: " + _name);
        return _s->peek();
    }
    void unget() {
        ensureOpen();
        if (_mode & std::ios_base::out)
            throw SxIOError("can't put char back into output stream: "
                            + _name);
        _s->unget();
    }
    std::istream& istream() { return *dynamic_cast<std::istream*>(_s); }
    std::ostream& ostream() { return *dynamic_cast<std::ostream*>(_s); }
    void print(Obj* x);
    void println(Obj* x);
    void print(const std::string& s);
    void println(const std::string& s);
    void finalize() {
        if (_s && _s->is_open()) {
            _s->close();
            delete _s;
            _s = nullptr;
        }
    }
protected:
    std::string _name;
    openMode_t _mode;
    std::fstream* _s;
    void ensureOpen() {
        if (!_s->is_open())
            throw SxIOError("file stream `" + _name + "' has been closed");
    }
    FStream(const std::string& name, openMode_t mode, std::fstream* s)
        : _name(name), _mode(mode), _s(s) {
        _typeName = "SxFStream";
    }
};
DEF_CASTER(FStream)

struct SStream : IInStream, IOutStream {
    static SStream* create(const std::string& initStr,
                           openMode_t mode = (std::ios_base::in
                                              | std::ios_base::out)) {
        std::stringstream* s = new std::stringstream(initStr, mode);
        return ::new (GC, finalizer) SStream(mode, s);
    }
    std::string toString() {
        std::stringstream ss;
        ss << "#<" << typeName() << openModeToStr(_mode) << ' '
           << this << '>';
        return ss.str();
    }
    openMode_t mode() { return _mode; }
    bool eof() { return _s->eof(); }
    void close();
    void put(int c) {
        if (!(_mode & std::ios_base::out))
            throw SxIOError("cant't write to input string stream");
        _s->put(c);
    }
    void flush() {
        if (!(_mode & std::ios_base::out))
            throw SxIOError("can't flush input string stream");
        _s->flush();
    }
    int get() {
        if (_mode & std::ios_base::out)
            throw SxIOError("can't read from output string stream");
        return _s->get();
    }
    int peek() {
        if (_mode & std::ios_base::out)
            throw SxIOError("can't read from output string stream");
        return _s->peek();
    }
    void unget() {
        if (_mode & std::ios_base::out)
            throw SxIOError("can't put char back into output string stream");
        _s->unget();
    }
    std::istream& istream() { return *dynamic_cast<std::istream*>(_s); }
    std::ostream& ostream() { return *dynamic_cast<std::ostream*>(_s); }
    void print(Obj* x);
    void println(Obj* x);
    void print(const std::string& s);
    void println(const std::string& s);
    void finalize() {
        if (_s) {
            delete _s;
            _s = nullptr;
        }
    }
protected:
    openMode_t _mode;
    std::stringstream* _s;
    SStream(openMode_t mode, std::stringstream* s)
        : _mode(mode), _s(s) {
        _typeName = "SxSStream";
    }
};
DEF_CASTER(SStream)

#endif // STREAM_HPP_INCLUDED
