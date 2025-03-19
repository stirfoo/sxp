/*
  list.cpp
  S. Edward Dolan
  Friday, April  7 2023
*/

#include "sxp.hpp"

Obj* List::EMPTY_LIST_MARKER = nullptr;

void List::init() {
    EMPTY_LIST_MARKER = Obj::create();
}

void List::setHead(Obj* x) {
    _head = x;
}

List* List::create() {
    return new List();
}

List* List::create(Obj* obj) {
    return new List(obj);
}

List* List::create(Obj* obj1, Obj* obj2) {
    return new List(obj1, obj2);
}

List* List::create(Obj* obj1, Obj* obj2, Obj* obj3) {
    return new List(obj1, obj2, obj3);
}

List* List::create(Obj* obj1, Obj* obj2, Obj* obj3, Obj* obj4) {
    return new List(obj1, obj2, obj3, obj4);
}

List* List::create(vecobj_t v) {
    ISeq* s = List::create();
    for (auto itr = v.rbegin(); itr!=v.rend(); ++itr)
        s = s->cons(*itr);
    return static_cast<List*>(s);
}

List::List() : _head(EMPTY_LIST_MARKER), _tail(NIL), _meta(nullptr) {
    _typeName = "SxList";
}

List::List(Obj* obj) : List() {
    _head = obj;
}

List::List(Obj* obj1, Obj* obj2) : List(obj1) {
    _tail = new List(obj2);
}

List::List(Obj* obj1, Obj* obj2, Obj* obj3) : List(obj1) {
    _tail = new List(obj2, obj3);
}

List::List(Obj* obj1, Obj* obj2, Obj* obj3, Obj* obj4) : List(obj1) {
    _tail = new List(obj2, obj3, obj4);
}

// =========================================================================
// IObj

std::string List::toString() {
    std::stringstream ss;
    ss << "(";
    for (ISeq* s=seq(); s; s=s->next()) {
        ss << rt::toString(s->first());
        if (s->next())
            ss << " ";
    }
    ss << ")";
    return ss.str();
}

size_t List::getHash() {
    std::stringstream ss;
    ss << _typeName << " is not hashable";
    throw SxRuntimeError(ss.str());
}

bool List::isEqualTo(Obj* obj) {
    if (this == obj)
        return true;
    else if (List* p = pList(obj)) {
        ISeq* s1 = seq(), *s2 = p->seq();
        for (; s1&&s2; s1=s1->next(), s2=s2->next())
            if (!rt::isEqualTo(s1->first(), s2->first()))
                return false;
        return !s1 && !s2;      // both lists consumed
    }
    else if (Vector* p = pVector(obj))
        return p->isEqualTo(this);
    else if (MapEntry* p = pMapEntry(obj)) {
        return count() == 2
            && rt::isEqualTo(first(), p->key())
            && rt::isEqualTo(rt::second(this), p->val());
    }
    return false;
}

List* List::copy() {
    List* lst = create(_head);
    lst->_tail = _tail;
    return lst;
}

// ========================================================================
// ISeq

Obj* List::first() {
    if (_head == EMPTY_LIST_MARKER)
        return NIL;
    return _head;
}

ISeq* List::rest() {
    if (_tail == NIL)
        return new List();
    return _tail;
}

ISeq* List::next() {
    return _tail;
}

ISeq* List::cons(Obj* obj) {
    List* ret = new List(obj);
    if (_head != EMPTY_LIST_MARKER)
        ret->_tail = this;
    return ret;
}

// =========================================================================
// ISeqable

ISeq* List::seq() {
    if (_head == EMPTY_LIST_MARKER)
        return nullptr;
    return this;
}

// =========================================================================
// IIndexed

Obj* List::nth(int i) {
    if (i >= 0) {
        int j=0;
        for (ISeq* s=seq(); s!=NIL; s=s->next(), ++j)
            if (j == i)
                return s->first();
    }
    std::stringstream ss;
    ss << _typeName << " index (" << i << ") out of bounds";
    throw SxOutOfBoundsError(ss.str());
}

Obj* List::nth(int i, Obj* notFound) {
    int j=0;
    for (ISeq* s=seq(); s!=NIL; s=s->next(), ++j)
        if (j == i)
            return s->first();
    return notFound;
}

// =========================================================================
// ICollection

int List::count() {
    int i=0;
    for (ISeq* s=seq(); s; s=s->next())
        ++i;
    return i;
}

bool List::isEmpty() {
    return _head == EMPTY_LIST_MARKER;
}

ICollection* List::conj(Obj* obj) {
    List* ret = new List(obj);
    if (!isEmpty())
        ret->_tail = this;
    return ret;
}

// =========================================================================
// IMeta

Hashmap* List::meta() {
    return _meta;
}

Obj* List::withMeta(Hashmap* m) {
    _meta = m;
    return this;
}
