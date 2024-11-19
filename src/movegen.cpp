#include "movegen.h"
#include "misc.h"
#include "types.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>


uint64_t xorshift::next() {
    state ^= state << 7; 
    state ^= state >> 9; 
    return state; 
}
uint64_t xorshift::sparse() { 
    return next() & next() & next(); 
}

unsigned Magic::index(bitboard all) {
    all &= rmask;
    unsigned t = unsigned((all * key) >> shamt);
    assert(t < (1U << (64 - shamt)));
    return t;
}
bitboard Magic::get_atts(bitboard all, bitboard us) {
    unsigned i = index(all);
    return atts[i] & ~us;
}

int t = 0;

Magic::Magic(int sqr, bool bsp, xorshift& ran) {
    rmask = movegen::rmask(sqr, bsp);
    int bitcount = __builtin_popcountll(rmask);
    int u = 1U << bitcount;
    shamt = 64 - bitcount;
    assert(bitcount <= 12);

    atts = new bitboard[4096];
    bitboard perms[4096], moves[4096];
    int iter[4096]{};

    for(int i = 0; i < u; ++i) {
        perms[i] = bits::occ_mask(i, bitcount, rmask); 
        moves[i] = movegen::satts(sqr, perms[i], bsp);
    }

    for(int i = 1; ; ++i, ++t) {
        do key = ran.sparse();
        while (!key);

        for(int j = 0; j < u; ++j) {
            unsigned k = unsigned((key * perms[j]) >> shamt);
            assert(k < unsigned(u));

            if( iter[k] < i ) {
                iter[k] = i;
                atts[k] = moves[j];
            }
            if( atts[k] != moves[j] ) 
                goto retry;
        }
        return;
        retry:;
    }
}




namespace movegen {

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

Magic magics[64][2];
bitboard patts_tbl[64][2], natts_tbl[64], katts_tbl[64];

void init_atts() {
    xorshift ran{0x2fe8c7467c13b5fb};
    for(int i = 0; i < 64; ++i) {
        magics[i][0] = Magic(i, 0, ran);
        magics[i][1] = Magic(i, 1, ran);

        patts_tbl[i][0] = patts(i, 0);
        patts_tbl[i][1] = patts(i, 1);
        natts_tbl[i] = natts(i);
        katts_tbl[i] = katts(i);
    }
}

bitboard ray(int sqr, int d, bitboard block) {
    bitboard bb = 0;

    const int df = dir_delta[d][0];
    const int dr = dir_delta[d][1];

    unsigned f = (sqr % 8) + df;
    unsigned r = (sqr / 8) + dr;
    for( ; f <= 7 && r <= 7; f += df, r += dr) {
        bitboard t = 1ULL << (8 * r + f);
        bb |= t;
        if( t & block ) break;
    }
    
    return bb;
}

bitboard rmask(int sqr, bool bsp) {
    bitboard mask = 0;
    constexpr bitboard r_bounds[4] {~rank_8, ~h_file, ~rank_1, ~a_file};
    constexpr bitboard b_bounds = ~(ah_file | rank_18);
    int offs = bsp ? 4: 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(sqr, dir, 0) & (bsp ? b_bounds : r_bounds[dir]);

    return mask;
}

bitboard satts(int sqr, uint64_t block, bool bsp) {
    bitboard mask = 0;
    int offs = bsp ? 4 : 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(sqr, dir, block);

    return mask;
}

bitboard patts(int sqr, bool turn) {
    bitboard mask = 0;
    constexpr int delta[2] {1, -1};
    unsigned f = sqr % 8;
    unsigned r = sqr / 8 + (turn ? 1 : -1);

    for(int df: delta)
        if( f + df <= 7 && r <= 7 ) 
            mask |= 1ULL << (8 * r + f + df);

    return mask;
}

bitboard natts(int sqr) {
    bitboard mask = 0;
    constexpr int delta[4] {-2, -1, 1, 2};
    unsigned f = sqr % 8;
    unsigned r = sqr / 8;

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df, rank = r + dr;
            if( abs(df) == abs(dr) || file > 7 || rank > 7 ) continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

    return mask;
}

bitboard katts(int sqr) {
    bitboard mask = 0;
    constexpr int delta[4] {-1, 0, 1};
    unsigned f = sqr % 8;
    unsigned r = sqr / 8;

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df, rank = r + dr;
            if( (!df && !dr) || file > 7 || rank > 7 ) continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

    return mask;
}

} // namespace movegen
