#ifndef BITS_H
#define BITS_H

#include "types.h"

#define POPCNT(t) ((u32)__builtin_popcountll(t))
#define TZCNT(t)  ((u32)__builtin_ctzll(t))

#define SQR_BB(s) ( 1ull << (s) )
#define SQR_FILE(s) ((s) % 8)
#define SQR_RANK(s) ((s) / 8)

#define GET_BIT(t, k) ( ((t) >> (k)) & 1 )
#define SET_BIT(t, k) ( (t) | (1ull << (k)) )
#define CLR_BIT(t, k) ( (t) & ~(1ull << (k)) )
#define ISO_LSB(t) ( (t) & -(t) )
#define POP_LSB(t) ( (t) & ((t) - 1) )

void print_bb(bitboard);

#endif
