#ifndef MOVEMAKE_H
#define MOVEMAKE_H

#include "types.h"
#include "position.h"
#include "move.h"
#include "movegen.h"

struct umake_dat {

    // position
    u32 en_passant_target;
    u32 hmoves;
    bool castle_k[2], castle_q[2];

    // move
    bitboard moved_bb;

    u32 cap_occupancy;
    u32 cap_sqr;      
    bitboard cap_bb;

    bitboard rk_bb;   

    bitboard promo_bb;
    u32 promo_type;   
};

void move_make(const struct atts_tbl*, struct position*, const move);

#endif
