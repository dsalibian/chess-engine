#ifndef MAGIC_H
#define MAGIC_H

#include "types.h"

struct magic {
    u64 magic;
    bitboard rmask;
    const bitboard* atts;
};

struct magic magic_make(const u32, const bool, u64*);
bitboard magic_moves_bb(const struct magic*, bitboard, bitboard);
void magics_init(struct magic*, struct magic*);

#endif
