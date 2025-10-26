#ifndef POSITION_H
#define POSITION_H

#include "types.h"

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
    u32 occupancy[64];

    u32 en_passant_target;
    u32 hmoves, fmoves;
    bool oo[2], ooo[2];
    bool turn;
};

void parse_fen(struct position*, const char*);
void startpos(struct position*);
void dbgprint_pos(const struct position*);

#endif
