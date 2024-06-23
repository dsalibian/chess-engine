#include <chrono>
#include <algorithm>
#include <iostream>

#include "movegen.h"
#include "position.h"
#include "misc.h"

using namespace std;

extern std::mt19937_64 eng;
extern std::uniform_int_distribution<unsigned long long> distr;
int main() {
    init_attacks();

    Position p { Position() };

    print_bitboard( pawn_moves(a2, p.all_pieces, p.b_pieces, W));

    return 0;
}
