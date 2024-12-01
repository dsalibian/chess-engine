#pragma once

#include "types.h"
#include <chrono>
#include <string>

namespace bits {

void print_bb(bitboard);

bool get_bit(bitboard, int);
void pop_lsb(bitboard&);
bitboard iso_lsb(bitboard);
bitboard occ_mask(unsigned, int, bitboard);

}

namespace ran {

uint64_t xorshift(uint64_t&);
uint64_t xorshift_sparse(uint64_t&);

}



std::string sqr_tostring(int);

template <typename F>
long time_func(F& f) {
    using namespace std::chrono;
    auto t0 = steady_clock::now();
    f();    
    return duration_cast<milliseconds>(steady_clock::now()-t0).count();
}
