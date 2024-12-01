#ifndef POSITION_H
#define POSITION_H

// make clangd shutup about nonsense recursive include error
#pragma once 

#include "types.h"
#include "movegen.h"
#include <string>

struct Position {
    bitboard bbs[12], w_bb, b_bb, all_bb;
    bitboard *us, *them;

    bool turn_w;
    bool oo_w, ooo_w, oo_b, ooo_b;
    unsigned en_passant;
    unsigned hmoves, fmoves;

    Position();
    Position(std::string);

    void parse_fen(std::string);
    void set_combined();

    Occupancy piece_at(int);

    void print(bool);
    void print_info();
    void print_full(bool);
};

#endif
