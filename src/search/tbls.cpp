#include "tbls.h"
#include <initializer_list>


namespace movegen {

enum Files_Ranks {
    a_file  = 0x0101010101010101ULL,
    h_file  = 0x8080808080808080ULL, 
    ah_file = 0x8181818181818181ULL, 

    rank_1  = 0x00000000000000ffULL,
    rank_8  = 0xff00000000000000ULL,
    rank_18 = 0xff000000000000ffULL,
};

enum Direction {
    north,
    east,
    south,
    west,
    neast,
    nwest,
    seast,
    swest,
};

constexpr int dir_delta[8][2] = {
    { 0,  1},
    { 1,  0},
    { 0, -1},
    {-1,  0},
    { 1,  1},
    {-1,  1},
    { 1, -1},
    {-1, -1},
};

bitboard ray(int pos, int d, bitboard block) {
    bitboard bb = 0;

    const int df = dir_delta[d][0];
    const int dr = dir_delta[d][1];

    unsigned f = (pos % 8) + df;
    unsigned r = (pos / 8) + dr;
    for( ; f <= 7 && r <= 7; f += df, r += dr) {
        bitboard t = 1ULL << (8 * r + f);
        bb |= t;
        if( t & block ) break;
    }
    
    return bb;
}

bitboard rmask(int pos, bool bsp) {
    bitboard mask = 0;
    constexpr bitboard r_bounds[4] {~rank_8, ~h_file, ~rank_1, ~a_file};
    constexpr bitboard b_bounds = ~(ah_file | rank_18);
    int offs = bsp ? 4: 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(pos, dir, 0) & (bsp ? b_bounds : r_bounds[dir]);

    return mask;
}

bitboard satts(int pos, uint64 block, bool bsp) {
    bitboard mask = 0;
    int offs = bsp ? 4 : 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(pos, dir, block);

    return mask;
}

} // namespace movegen



namespace bits {

bool get_bit(bitboard bb, int k) {
    return (bb >> k) & 1;
}

void pop_lsb(bitboard& bb) {
    bb &= bb - 1; 
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

uint64 next_lperm(uint64 v) {
    uint64 t = v | (v - 1);
    return (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctzll(v) + 1));
}

} // namespace bits



enum Rk_Bsp {rook, bishop};
Tbls::Tbls() {
    rmask  = new bitboard*[64];
    popcnt = new int*[64];
    u      = new int*[64];
    shamt  = new int*[64];
    perms  = new bitboard**[64];
    moves  = new bitboard**[64];

    for(int s = 0; s < 64; ++s) {
        rmask[s]  = new bitboard[2];
        popcnt[s] = new int[2];
        u[s]      = new int[2];
        shamt[s]  = new int[2];
        perms[s]  = new bitboard*[2];
        moves[s]  = new bitboard*[2];

        for( int t: {rook, bishop} ) {
            rmask [s][t] = movegen::rmask(s, t);
            popcnt[s][t] = __builtin_popcountll(rmask[s][t]);
            u     [s][t] = 1U << popcnt[s][t];
            shamt [s][t] = 64  - popcnt[s][t];
            perms [s][t] = new bitboard[u[s][t]];
            moves [s][t] = new bitboard[u[s][t]];
        }

        for(int i = 0; i < u[s][rook]; ++i) {
            for( int t: {rook, bishop} ) {
                if( i < u[s][t] ) {
                    perms[s][t][i] = bits::occ_mask(i, popcnt[s][t], rmask[s][t]);
                    moves[s][t][i] = movegen::satts(s, perms[s][t][i], t);
                }
            }
        }
    }
}
