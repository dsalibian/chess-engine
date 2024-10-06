#ifndef POSITION_H
#define POSITION_H

#include "types.h"

struct Position {
    bitboard bbs[12], w_bb, b_bb, all_bb;
    bitboard* us;
    bitboard* them;

    bool turn;
    bool oo_w, ooo_w, oo_b, ooo_b;
    unsigned ep_target;

    Position();
};


#endif
