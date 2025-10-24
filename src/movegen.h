#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"

bitboard gen_ratts(const u32, const bitboard);
bitboard gen_batts(const u32, const bitboard);
bitboard gen_patts(const u32, const bool);
bitboard gen_natts(const u32);
bitboard gen_katts(const u32);

bitboard gen_rmask(const u32, const bool);

#endif
