#pragma once

#include <cstdint>
#include <cstddef>

class Bitset {
public:
    int64_t x;
    explicit Bitset(int64_t x_ = 0):x(x_){}
    void insert(size_t i){
        x |= int64_t(1) << i;
    }
    void remove(size_t i){
        x &= ~(int64_t(1) << i);
    }
    bool contains(size_t i) const {
        return x & (int64_t(1) << i);
    }
    Bitset join(const Bitset &s) const {
        return Bitset{x | s.x};
    }
    Bitset except(const Bitset &s) const {
        return Bitset{x & ~s.x};
    }
    Bitset intersect(const Bitset &s) const {
        return Bitset{x & s.x};
    }
    Bitset begin() const {
        return Bitset{x & (-x)};
    }
    const Bitset &end() const {
        return *this;
    }
    void next(const Bitset &S) {
        x = S.x & (x - S.x);
    }

    bool operator<(const Bitset &b) const {
        return x < b.x;
    }
    bool operator==(const Bitset &b) const {
        return x == b.x;
    }
    bool operator!=(const Bitset &b) const {
        return !(*this == b);
    }
    static Bitset emptySet(){
        return Bitset{0};
    }
    static Bitset fullSet(const size_t &N) {
        return Bitset{(int64_t(1) << N) - 1};
    }
};
