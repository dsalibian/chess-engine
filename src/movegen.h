#ifndef MOVEGEN_H
#define MOVEGEN_H

// make clangd shutup about nonsense recursive include error
#pragma once

#include "types.h"
#include "position.h"
#include "misc.h"

struct MoveGen {
    Magic magics[64][2];
    bitboard patts[64][2], natts[64], katts[64];

    MoveGen();

    bitboard pmoves(int, bool, bitboard, bitboard);
    bitboard nmoves(int, bitboard);
    bitboard bmoves(int, bitboard, bitboard);
    bitboard rmoves(int, bitboard, bitboard);
    bitboard qmoves(int, bitboard, bitboard);
    bitboard kmoves(int, bitboard);

    bool attd(int, bool, bitboard, bitboard, bitboard, bitboard, bitboard, bitboard, bitboard);
};

namespace movegen {

bitboard next_move(bitboard&);

bitboard ray(int, int, bitboard);
bitboard rmask(int, bool);
bitboard satts(int, bitboard, bool);
bitboard patts(int, bool);
bitboard natts(int);
bitboard katts(int);

} // namespace movegen

#endif
