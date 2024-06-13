#ifndef POSITION_H
#define POSITION_H

using namespace std;
typedef unsigned long long ull;
typedef unsigned short ushort;

#define BB(n) ( 1ULL << n )

const ull a_file  = 0x0101010101010101ULL;
const ull b_file  = 0x0202020202020202ULL;
const ull g_file  = 0x4040404040404040ULL;
const ull h_file  = 0x8080808080808080ULL;
const ull ab_file = 0x303030303030303ULL;
const ull gh_file = 0xc0c0c0c0c0c0c0c0ULL;

const ull rank_1  = 0x00000000000000ffULL;
const ull rank_2  = 0x000000000000ff00ULL;
const ull rank_7  = 0x00ff000000000000ULL;
const ull rank_8  = 0xff00000000000000ULL;
const ull rank_12 = 0x000000000000ffffULL;
const ull rank_78 = 0xffff000000000000ULL;

enum Squares{ 
    a1, b1, c1, d1, e1, f1, g1, h1, 
    a2, b2, c2, d2, e2, f2, g2, h2, 
    a3, b3, c3, d3, e3, f3, g3, h3, 
    a4, b4, c4, d4, e4, f4, g4, h4, 
    a5, b5, c5, d5, e5, f5, g5, h5, 
    a6, b6, c6, d6, e6, f6, g6, h6, 
    a7, b7, c7, d7, e7, f7, g7, h7, 
    a8, b8, c8, d8, e8, f8, g8, h8, 
};

enum Piece {
    empty,

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
};

enum Side {
    W,
    B
};

#endif