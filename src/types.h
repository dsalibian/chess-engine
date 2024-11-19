#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>
#include <array>

using std::uint64_t;
using bitboard = std::uint64_t;

enum Occupancy {
    w_pawn,
    w_night,
    w_bishop,
    w_rook,
    w_queen,
    w_king,

    b_pawn,
    b_night,
    b_bishop,
    b_rook,
    b_queen,
    b_king,

    empty,
};

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1, 
    a2, b2, c2, d2, e2, f2, g2, h2, 
    a3, b3, c3, d3, e3, f3, g3, h3, 
    a4, b4, c4, d4, e4, f4, g4, h4, 
    a5, b5, c5, d5, e5, f5, g5, h5, 
    a6, b6, c6, d6, e6, f6, g6, h6, 
    a7, b7, c7, d7, e7, f7, g7, h7, 
    a8, b8, c8, d8, e8, f8, g8, h8, 
};

#define SQ_BB(file, rank) file##rank##_bb = 1ULL << file##rank,
#define RANK_BBS(rank) SQ_BB(a, rank) SQ_BB(b, rank) SQ_BB(c, rank) SQ_BB(d, rank) \
                       SQ_BB(e, rank) SQ_BB(f, rank) SQ_BB(g, rank) SQ_BB(h, rank)

enum Square_BB {
    RANK_BBS(1)
    RANK_BBS(2)
    RANK_BBS(3)
    RANK_BBS(4)
    RANK_BBS(5)
    RANK_BBS(6)
    RANK_BBS(7)
    RANK_BBS(8)
};

enum Files_Ranks {
    a_file  = 0x0101010101010101ULL,
    b_file  = 0x0202020202020202ULL, 
    g_file  = 0x4040404040404040ULL, 
    h_file  = 0x8080808080808080ULL, 
    ab_file = 0x0303030303030303ULL, 
    gh_file = 0xc0c0c0c0c0c0c0c0ULL, 
    ah_file = 0x8181818181818181ULL, 

    rank_1  = 0x00000000000000ffULL,
    rank_2  = 0x000000000000ff00ULL,
    rank_7  = 0x00ff000000000000ULL,
    rank_8  = 0xff00000000000000ULL,
    rank_12 = 0x000000000000ffffULL,
    rank_78 = 0xffff000000000000ULL,
    rank_18 = 0xff000000000000ffULL,
};

#endif
