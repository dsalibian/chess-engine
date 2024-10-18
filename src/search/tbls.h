#ifndef TBLS_H
#define TBLS_H

#include <cstdint>

using uint64   = uint64_t;
using bitboard = uint64_t;


namespace movegen {

bitboard ray(int, int, bitboard);
bitboard rmask(int, bool);

} // namespace movegen


namespace bits {

bool get_bit(bitboard, int);
void pop_lsb(bitboard&);
bitboard occ_mask(unsigned, int, bitboard);

// https://graphics.stanford.edu/%7Eseander/bithacks.html#NextBitPermutation
uint64 next_lperm(uint64);

} // namespace bits


struct Tbls {
    bitboard**  rmask;
    int**       popcnt;
    int**       shamt;
    int**       u;
    bitboard*** perms;
    bitboard*** moves;

    Tbls();
};

#endif
