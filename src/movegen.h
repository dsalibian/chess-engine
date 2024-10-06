#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "position.h"
#include "misc.h"

struct Magic {
    uint64    key;
    bitboard* atts;
    bitboard  rmask;
    unsigned  shamt;
    Magic(int, bool, xorshift&);
    Magic(){};
    unsigned index(bitboard all) {
        all &= rmask;
        unsigned t = unsigned((all * key) >> shamt);
        assert(t < (1U << (64 - shamt)));
        return t;
    }
    bitboard get_atts(bitboard all, bitboard us) {
        unsigned i = index(all);
        return atts[i] & ~us;
    }
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
