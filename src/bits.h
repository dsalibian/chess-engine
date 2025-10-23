#ifndef BITS_H
#define BITS_H

#include "types.h"

#define SQR_BB(s) ( 1ull << (s) )

#define GET_BIT(t, k) ( ((t) >> (k)) & 1 )
#define SET_BIT(t, k) ( (t) | (1ull << (k)) )
#define CLR_BIT(t, k) ( (t) & ~(1ull << (k)) )
#define ISO_LSB(t) ( (t) & -(t) )
#define POP_LSB(t) ( (t) & ((t) - 1) )

void print_bb(bitboard);

#endif
