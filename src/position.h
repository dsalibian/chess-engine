#ifndef POSITION_H
#define POSITION_H

#include "types.h"
#include "move.h"

#define TURN_B false
#define TURN_W true

enum bb_type_idx {
    IDX_PAWN, 
    IDX_NIGHT,
    IDX_BISHP,
    IDX_ROOK,
    IDX_QUEEN,
    IDX_KING,

    IDX_ALL,
};

struct position {
    bitboard bbs[2][7], bb_all;
    u8 board[64];

    u32 hmoves, fmoves;
    u8 en_passant_target;
    bool castle_k[2], castle_q[2];
    bool turn;
};

struct atts_tbl;

void pos_fen(struct position*, const char*);
void pos_startpos(struct position*);
void pos_print(const struct position*);

#endif
