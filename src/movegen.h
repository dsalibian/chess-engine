#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "magic.h"
#include "position.h"
#include "move.h"

bitboard gen_satts(const u32, const bool, const bitboard);
bitboard gen_ratts(const u32, const bitboard);
bitboard gen_batts(const u32, const bitboard);
bitboard gen_patts(const u32, const bool);
bitboard gen_natts(const u32);
bitboard gen_katts(const u32);

bitboard gen_rmask(const u32, const bool);

struct atts_tbl {
    // attack bitboard tables
    bitboard patts[2][64], ppush[2][64], dpush[2][64], dpush_check[2][64];
    bitboard natts[64], katts[64];

    struct magic rk_magic[64], bsp_magic[64];

    bitboard castle_r_bb[2][2];

    u32 castle_r_to[2][2];
    u32 castle_r_from[2][2];

    bitboard between_bb[64][64];
    bitboard through_bb[64][64];
};

struct atts_tbl gen_atts_tbl();

bitboard bmoves_bb(const struct atts_tbl*, const u32, const bitboard, const bitboard);
bitboard rmoves_bb(const struct atts_tbl*, const u32, const bitboard, const bitboard);
bitboard qmoves_bb(const struct atts_tbl*, const u32, const bitboard, const bitboard);

bitboard nmoves_bb(const struct atts_tbl*, const u32, const bitboard);
bitboard kmoves_bb(const struct atts_tbl*, const u32, const bitboard);

bitboard pmoves_push_bb(const struct atts_tbl*, const u32, const bitboard, const bool);
bitboard pmoves_dpush_bb(const struct atts_tbl*, const u32, const bitboard, const bool);
bitboard pmoves_cap_bb(const struct atts_tbl*, const u32, const bitboard, const bool);
bitboard pmoves_ep_bb(const struct atts_tbl*, const u32, const u32, const bool);

void genmoves_legal(
        const struct atts_tbl*,
        struct move_stack*,
        const bool,
        const bool,
        const bool,
        const u32,
        const bitboard,
        const bitboard[2][7]);


#endif
