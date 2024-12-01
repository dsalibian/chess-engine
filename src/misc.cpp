#include "misc.h"
#include <iostream>
#include <cassert>

namespace bits {



void print_bb(bitboard bb) {
    for(int r = 7; r >= 0; --r) {
        std::cout << ' ' << r + 1 << "   ";
        for(int f = 0; f <= 7; ++f) 
            std::cout << (get_bit(bb, 8 * r + f) ? "1 " : ". ");
        std::cout << '\n';
    }
    std::cout << "\n     a b c d e f g h\n\n\n";
}

bool get_bit(bitboard bb, int k) {
    return (bb >> k) & 1;
}

void pop_lsb(bitboard& bb) {
    bb &= bb - 1; 
}

bitboard iso_lsb(bitboard bb) {
    return bb & -bb; 
}

bitboard occ_mask(unsigned index, int bitcount, bitboard atts) {
    bitboard occ = 0;

    for(int i = 0; i < bitcount; ++i) {
        int k = __builtin_ctzll(atts);
        pop_lsb(atts);

        if( index & (1U << i)) 
            occ |= (1ULL << k);
    }

    return occ;
}

} // namespace bits



namespace ran {

uint64_t xorshift(uint64_t& state) {
    state ^= state << 7;
    state ^= state >> 9;
    return state;
}

uint64_t xorshift_sparse(uint64_t& state) {
    return xorshift(state) & xorshift(state) & xorshift(state);
}

} // namespace ran



std::string sqr_tostring(int sqr) {
    return std::string {char('a' + (sqr % 8)), char('1' + sqr / 8)};
}
