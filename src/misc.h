#ifndef MISC_H
#define MISC_H

#include "types.h"

char occupancy_char(const u32);
void print_occupancy(const u32*); 

void build_occupancy(u32*, const bitboard*);

#endif
