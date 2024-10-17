#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"
#include "misc.h"

struct xorshift;
struct Magic;

namespace movegen {

void init_atts();
bitboard occ_mask(unsigned, int, bitboard);
bitboard ray(int, int, bitboard);
bitboard rmask(int, bool);
bitboard satts(int, bitboard, bool);
bitboard patts(int, bool);
bitboard natts(int);
bitboard katts(int);

}


#endif
