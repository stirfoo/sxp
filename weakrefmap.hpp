/*
  weakrefmap.hpp
  S. Edward Dolan
  Thursday, April 20 2023
*/

#ifndef WEAKREFMAP_HPP_INCLUDED
#define WEAKREFMAP_HPP_INCLUDED

template <typename T>
struct WeakRefMap {
    void clear() { expire(); _m.clear(); }
    T* fetch(const std::string& s, bool isPointerFree = true) {
        T* obj;
        auto itr = _m.find(s);
        if (itr == _m.end()) {
            // not in the cache
            if (_expireCount++ > EXPIRE_THRESHOLD) {
                expire();
                _expireCount = 0;
            }
            if (isPointerFree)
                obj = new (PointerFreeGC) T(s);
            else
                obj = new T(s);
            _m[s] = newWeakRef(obj);
        }
        else {
            WeakRef* ref = itr->second;
            if (ref && ref->_data)
                // hit!
                obj = static_cast<T*>(ref->_data);
            else {
                // cached Keyword got collected... do it again
                _m.erase(itr);
                obj = fetch(s);
            }
        }
        return obj;
    }
    const auto& m() { return _m; }
protected:    
    int _expireCount;
    // for every N new obj added to the cache, delete all unrooted objects
    static constexpr int EXPIRE_THRESHOLD = 32;
    struct WeakRef {
        void* _data;
    };
    std::map<const std::string,
             WeakRef*,
             std::less<std::string>,
             gc_allocator<std::pair<const std::string,
                                    WeakRef*>>> _m;
    static WeakRef* newWeakRef(void* data) {
        WeakRefMap::WeakRef* ref = new (PointerFreeGC) WeakRef();
        ref->_data = data;
        int result = GC_general_register_disappearing_link(&ref->_data,
                                                           ref->_data);
        if (result)
            throw SxRuntimeError("WeakRefMap: GC_DUPLICATE or"
                                 " GC_NO_MEMORY");
        return ref;
    }
    void expire() {
        auto itr = _m.begin(), END = _m.end();
        while (itr != END) {
            if (!itr->second || !itr->second->_data)
                _m.erase(itr++);
            else
                ++itr;
        }
    }
};

#endif // WEAKREFMAP_HPP_INCLUDED
