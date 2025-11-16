#ifndef MAGIC_H
#define MAGIC_H

#include "types.h"

#define MAGIC_HASH(magic, relevant_occupancy_bb, popcnt) (((magic) * (relevant_occupancy_bb)) >> (64u - popcnt))

struct magic {
    u64 magic;
    bitboard rmask;
    const bitboard* atts;
};

struct magic magic_make(const u32, const bool, u64*);

bitboard magic_moves_bb(const struct magic*, const bitboard, const bitboard);

#endif
