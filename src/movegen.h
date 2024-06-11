#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <array>
#include <bits/stdc++.h>
/*
    __builtin_popcountll()
    ffsll()
    __builtin_clzll()
    __builtin_ctzll()
*/
 
#include "position.h"
#include "misc.h"

using namespace std;
typedef unsigned long long ull;

void init_attacks();

ull find_magic(int, int, bool);
ull relevant_occupancy_mask(int, int, ull);
ull transform(ull, ull, int);

ull pawn_attacks_mask(int, bool);
ull night_attacks_mask(int);
ull bishop_relevant_mask(int);
ull rook_relevant_mask(int);
ull bishop_attacks_mask(int, ull);
ull rook_attacks_mask(int, ull);
ull queen_attacks_mask(int);
ull king_attacks_mask(int);

#endif