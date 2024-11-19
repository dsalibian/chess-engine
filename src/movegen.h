#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"
#include "misc.h"

struct xorshift {
    uint64_t state;
    uint64_t next();
    uint64_t sparse();
};

struct Magic {
    uint64_t  key;
    bitboard* atts;
    bitboard  rmask;
    unsigned  shamt;

    Magic(int, bool, xorshift&);
    Magic(){};
    unsigned index(bitboard);
    bitboard get_atts(bitboard, bitboard);
};

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
