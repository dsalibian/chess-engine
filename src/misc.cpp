#include "misc.h"
#include <iostream>
#include <cassert>

void print_bb(bitboard bb) {
    using std::cout;

    for(int r = 7; r >= 0; --r) {
        cout << ' ' << r + 1 << "   ";
        for(int f = 0; f <= 7; ++f) 
            cout << (get_bit(bb, 8 * r + f) ? "1 " : ". ");
        cout << '\n';
    }
    cout << "\n     a b c d e f g h\n\n\n";
}

bool get_bit(bitboard bb, int k) {
    assert(k < 64);
    return (bb >> k) & 1;
}

void pop_lsb(bitboard& bb) {
    bb &= bb - 1; 
}

bitboard lsb_bb(bitboard bb) {
    return bb & -bb; 
}

bitboard occ_mask(unsigned index, int bitcount, bitboard atts) {
    assert(index < (1U << bitcount));
    bitboard occ = 0;

    for(int i = 0; i < bitcount; ++i) {
        int k = __builtin_ctzll(atts);
        pop_lsb(atts);

        if( index & (1U << i)) 
            occ |= (1ULL << k);
    }

    return occ;
}
