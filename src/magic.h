#ifndef MAGIC_H
#define MAGIC_H

#include "types.h"

#define MAGIC_HASH(magic, relevant_occupancy, shamt) (((magic) * (relevant_occupancy)) >> (shamt))

struct magic {
    u64 magic;
    bitboard rmask;
    const bitboard* atts;
};

struct magic magic_make(const u32, const bool, u64*);
void magics_init(struct magic*, struct magic*);

bitboard magic_moves_bb(const struct magic*, const bitboard, const bitboard);

#endif
