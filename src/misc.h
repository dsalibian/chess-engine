#ifndef MISC_H
#define MISC_H

#include <iostream>
#include <chrono>

/*
    __builtin_popcountll() - count 1 bits
    ffsll()  - return lsb index ( index starts at 1, returns 0 if n == 0 )
    __builtin_clzll()   Count Leading Zeros
    __builtin_ctzll()   Count Trailing Zeros
*/
#define bit_count(n) __builtin_popcountll(n)
#define lsb_index(n) __builtin_ctzll(n) 

using namespace std;
typedef unsigned long long ull;

void print_bitboard(ull);
bool get_bit(ull, int);
void set_bit(ull&, int, bool);
int pop_lsb(ull&);
ull BB(int);
int f(ull, ull, int);

template <typename F>
long time_func(F& f) {
    auto t0 = chrono::steady_clock::now();
    f();    
    return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-t0).count();
}

#endif
