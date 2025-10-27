#ifndef MISC_H
#define MISC_H

#include "types.h"
#include "movegen.h"

#define OCCUPANCY_ISWHITE(o)    ((u32)o < OCCUPANCY_B_PAWN)
#define OCCUPANCY_ISBLACK(o)    ((o) > OCCUPANCY_W_KING && (o) < OCCUPANCY_EMPTY)
#define OCCUPANCY_TYPE(o) \
    ((o) == OCCUPANCY_EMPTY ? TYPE_NONE : ((o) - ((o) > OCCUPANCY_W_KING ? OCCUPANCY_B_PAWN : 0)))

#define TYPE_OCCUPANCY(type, side) \
    ((type) == TYPE_NONE ? OCCUPANCY_EMPTY : ((type) + ((side) ? 0 : OCCUPANCY_B_PAWN)))

char occupancy_char(const u32);
void print_occupancy(const u32*); 

bool sqr_attd_opps(const struct atts_tbl*, const u32, const bitboard, const bitboard*, const bool);

#endif
