#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "magic.h"

bitboard gen_satts(const u32, const bool, const bitboard);
bitboard gen_ratts(const u32, const bitboard);
bitboard gen_batts(const u32, const bitboard);
bitboard gen_patts(const u32, const bool);
bitboard gen_natts(const u32);
bitboard gen_katts(const u32);

bitboard gen_rmask(const u32, const bool);




struct movegen_tbl {
    bitboard patts[2][64], natts[64], katts[64];
    struct magic rk_magic[64], bsp_magic[64];
};

struct movegen_tbl gen_mg_tbl();

#endif
