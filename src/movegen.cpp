#include "movegen.h"
#include "misc.h"
#include "types.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int magic_search_iters = 0;



Magic::Magic(int sqr, bool bsp, uint64_t& ran_state) {
    int popcnt, u;

    rmask = movegen::rmask(sqr, bsp);
    popcnt = __builtin_popcountll(rmask);
    u = 1u << popcnt;
    shamt = 64 - popcnt;

    atts = new bitboard[4096];
    bitboard perms[4096], moves[4096];
    unsigned iter[4096]{};

    for(int i = 0; i < u; ++i) {
        perms[i] = bits::occ_mask(i, popcnt, rmask); 
        moves[i] = movegen::satts(sqr, perms[i], bsp);
    }

    for(unsigned i = 1; ; ++i, ++magic_search_iters) {
        do magic = ran::xorshift_sparse(ran_state);
        while (!magic);

        for(int j = 0; j < u; ++j) {
            unsigned k = unsigned((magic * perms[j]) >> shamt);

            if(iter[k] < i) {
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

// omitting us parameter returns squares seen, including friendly pieces
bitboard Magic::get_moves(bitboard all, std::optional<bitboard> us = ~0ULL) {
    bitboard key = all & rmask;
    unsigned index = unsigned((magic * key) >> shamt); 
    return atts[index] & *us;
}

MoveGen::MoveGen() {
    uint64_t ran_state = 0x2fe8c7467c13b5fbull;

    for(int sqr = 0; sqr < 64; ++sqr) {
        magics[sqr][0] = Magic(sqr, 0, ran_state);
        magics[sqr][1] = Magic(sqr, 1, ran_state);

        patts[sqr][0] = movegen::patts(sqr, 0);
        patts[sqr][1] = movegen::patts(sqr, 1);
        natts[sqr] = movegen::natts(sqr);
        katts[sqr] = movegen::katts(sqr);
    }
}

bitboard MoveGen::pmoves(int sqr, bool white, bitboard all, bitboard them) {
    bitboard mask = 0;

    bitboard sqr_bb = 1u << sqr;
    bitboard spush = white ? sqr_bb << 8 : sqr_bb >> 8;
    bitboard dpush = spush | (white ? (spush << 8) : spush >> 8);

    if(!(all & spush))
        mask |= spush;

    if((sqr_bb & (white ? rank_2 : rank_7)) && !(all & dpush))
        mask |= dpush;

    mask |= patts[sqr][white] & them;

    return mask;
}

bitboard MoveGen::nmoves(int sqr, bitboard us) {
    return natts[sqr] & ~us;
}

bitboard MoveGen::rmoves(int sqr, bitboard all, bitboard us) {
    return magics[sqr][0].get_moves(all, us);
}

bitboard MoveGen::bmoves(int sqr, bitboard all, bitboard us) {
    return magics[sqr][1].get_moves(all, us);
}

bitboard MoveGen::qmoves(int sqr, bitboard all, bitboard us) {
    return rmoves(sqr, all, us) | bmoves(sqr, all, us);
}

bitboard MoveGen::kmoves(int sqr, bitboard us) {
    return katts[sqr] & ~us;
}

bool MoveGen::attd(int sqr, bool white, bitboard p, bitboard n, bitboard b,
        bitboard r, bitboard q, bitboard k, bitboard all)
{
    bitboard ratts = magics[sqr][0].get_moves(all);
    bitboard batts = magics[sqr][1].get_moves(all);
    bitboard qatts = ratts | batts;

    return  (p & patts[sqr][white]) || 
            (n & natts[sqr]) ||
            (k & katts[sqr]) || 
            (b & batts) || 
            (r & ratts) ||
            (q & qatts);
}





namespace movegen {

bitboard next_move(bitboard& moves_mask) {
    bitboard m = bits::iso_lsb(moves_mask);
    bits::pop_lsb(moves_mask);

    return m;
}

enum Direction {
    north, east,  south,  west,
    neast, nwest, seast, swest,
};

constexpr int dir_delta[8][2] = {
    {0, 1}, { 1, 0}, {0, -1}, {-1,  0},
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
};

bitboard ray(int sqr, int d, bitboard block) {
    bitboard bb = 0;
    const int df = dir_delta[d][0];
    const int dr = dir_delta[d][1];
    unsigned f = (sqr % 8) + df;
    unsigned r = (sqr / 8) + dr;

    for(; f <= 7 && r <= 7; f += df, r += dr) {
        bitboard t = 1ULL << (8 * r + f);
        bb |= t;
        if(t & block) 
            break;
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

bitboard patts(int sqr, bool white) {
    bitboard mask = 0;
    unsigned f = sqr % 8;
    unsigned r = sqr / 8 + (white ? 1 : -1);

    for(int df: {1, -1})
        if(f + df <= 7 && r <= 7) 
            mask |= 1ULL << (8 * r + f + df);

    return mask;
}

bitboard natts(int sqr) {
    bitboard mask = 0;
    unsigned f = sqr % 8;
    unsigned r = sqr / 8;
    constexpr int delta[4] {-2, -1, 1, 2};

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df;
            unsigned rank = r + dr;
            if(abs(df) == abs(dr) || file > 7 || rank > 7) 
                continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

    return mask;
}

bitboard katts(int sqr) {
    bitboard mask = 0;
    unsigned f = sqr % 8;
    unsigned r = sqr / 8;
    constexpr int delta[4] {-1, 0, 1};

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df, rank = r + dr;
            if((!df && !dr) || file > 7 || rank > 7) 
                continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

    return mask;
}

} // namespace movegen
