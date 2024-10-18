#ifndef MISC_H
#define MISC_H

#include "types.h"
#include <chrono>

namespace bits {

void print_bb(bitboard);

bool get_bit(bitboard, int);
void pop_lsb(bitboard&);
bitboard lsb_bb(bitboard);
bitboard occ_mask(unsigned, int, bitboard);

}

template <typename F>
long time_func(F& f) {
    using namespace std::chrono;
    auto t0 = steady_clock::now();
    f();    
    return duration_cast<milliseconds>(steady_clock::now()-t0).count();
}

#endif
