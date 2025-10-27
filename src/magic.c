#include "magic.h"
#include "bits.h"
#include "movegen.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

bitboard nth_occupancy_mask(bitboard rmask, const u32 n) {
    const u32 popcnt = POPCNT(rmask);
    bitboard occupancy  = 0;

    for(u32 i = 0; i < popcnt; ++i) {
        u32 shamt = TZCNT(rmask);
        rmask = POP_LSB(rmask);
       
        if(n & (1u << i))
            occupancy |= 1ull << shamt;
    }

    return occupancy;
}

u64 xorshift(u64* state) {
    *state ^= *state << 7;
    *state ^= *state >> 9;

    return *state;
}

struct magic magic_make(const u32 sqr, const bool bsp, u64* s) {
    const bitboard rmask = gen_rmask(sqr, bsp);
    const u32 popcnt     = POPCNT(rmask);
    const u32 u          = 1u << popcnt;

    bitboard* atts = malloc(u * sizeof(bitboard));
    bitboard occupancy_masks[u],  reference_atts[u];

    for(u32 i = 0; i < u; ++i) {
        occupancy_masks[i] = nth_occupancy_mask(rmask, i);
        reference_atts[i]  = gen_satts(sqr, bsp, occupancy_masks[i]);
    }

    u32 iter[u];
    memset(iter, 0, u * sizeof(*iter));

    for(u32 i = 1; ; ++i) {
        u64 magic = xorshift(s) & xorshift(s) & xorshift(s);

        for(u32 j = 0; j < u; ++j) {
            u64 idx = MAGIC_HASH(magic, occupancy_masks[j], popcnt);
            assert(idx < u);

            if(iter[idx] != i) {
                iter[idx] = i;
                atts[idx] = reference_atts[j];
            } else if(atts[idx] != reference_atts[j])
                goto retry;
        }

        return (struct magic){.magic = magic, .rmask = rmask, .atts = atts};
        retry:;
    }
}

bitboard magic_moves_bb(const struct magic* magic, const bitboard all, const bitboard us) {
    const u64 i = MAGIC_HASH(magic->magic, all & magic->rmask, POPCNT(magic->rmask));
    assert(i < 1u << POPCNT(magic->rmask));

    return magic->atts[i] & ~us;
}
