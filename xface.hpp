/*
  xface.hpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#ifndef XFACE_HPP_INCLUDED
#define XFACE_HPP_INCLUDED

struct Obj;
struct Hashmap;

struct IMeta : virtual Obj {
    // TODO: bool hasMeta() = 0;
    virtual Hashmap* meta() = 0;
    virtual Obj* withMeta(Hashmap*) = 0;
};
DEF_CASTER(IMeta)

struct ISeq : virtual Obj {
    virtual Obj* first() = 0;
    virtual ISeq* rest() = 0;
    virtual ISeq* next() = 0;
    virtual ISeq* cons(Obj*) = 0;
};
DEF_CASTER(ISeq)

struct ISeqable : virtual Obj {
    virtual ISeq* seq() = 0;
};
DEF_CASTER(ISeqable)

struct INumber : virtual Obj {
    virtual long toInt() = 0;
    virtual double toFloat() = 0;
    virtual INumber* add(INumber*) = 0;
    virtual INumber* sub(INumber*) = 0;
    virtual INumber* mul(INumber*) = 0;
    virtual INumber* div(INumber*) = 0;
    virtual INumber* neg() = 0;
    virtual bool eq(INumber*) = 0; // (== x & xs) numerical equality
    virtual bool lt(INumber*) = 0;
};
DEF_CASTER(INumber)

struct IIndexed : virtual Obj {
    virtual Obj* nth(int) = 0;
    virtual Obj* nth(int, Obj*) = 0;
};
DEF_CASTER(IIndexed)

struct ICollection : virtual Obj {
    virtual int count() = 0;
    virtual bool isEmpty() = 0;
    virtual ICollection* conj(Obj*) = 0;
};
DEF_CASTER(ICollection)

struct MapEntry;

struct IAssociative : virtual Obj {
    virtual IAssociative* assoc(Obj*, Obj*) = 0;
    virtual IAssociative* dissoc(Obj*) = 0;
    virtual bool hasKey(Obj*) = 0;
    virtual MapEntry* entryAt(Obj*) = 0;
    virtual Obj* valAt(Obj*, Obj* notFound=NIL) = 0;
};
DEF_CASTER(IAssociative)

struct ISortable : virtual Obj {
    virtual bool less(Obj*) = 0;
};
DEF_CASTER(ISortable)

// =========================================================================
// IO Streams

typedef std::ios_base::openmode openMode_t;

struct IStream : virtual Obj {
    virtual openMode_t mode() = 0;
    virtual bool eof() = 0;
    virtual void close() = 0;
};
DEF_CASTER(IStream)

struct IInStream : IStream {
    virtual int get() = 0;
    virtual int peek() = 0;
    virtual void unget() = 0;
    virtual std::istream& istream() = 0;
};
DEF_CASTER(IInStream)

struct IOutStream : IStream {
    virtual void put(int c) = 0;
    virtual void flush() = 0;
    virtual void print(Obj*) = 0;
    virtual void println(Obj*) = 0;
    virtual void print(const std::string&) = 0;
    virtual void println(const std::string&) = 0;
    virtual std::ostream& ostream() = 0;
};
DEF_CASTER(IOutStream)

struct ISet : virtual Obj {
    virtual ISet* disjoin(Obj*) = 0;
    virtual bool contains(Obj*) = 0;
    virtual Obj* get(Obj*, Obj* notFound=NIL) = 0;
};
DEF_CASTER(ISet)

#endif // XFACE_HPP_INCLUDED
