#ifndef POSITION_H
#define POSITION_H

typedef unsigned long long ull;
typedef unsigned short ushort;

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
