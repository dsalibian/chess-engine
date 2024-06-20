#include <iostream>

#include "position.h"
#include "movegen.h"

using namespace std;

int main() {
    print_bitboard( rook_relevant_mask(a5));

    return 0;
    ull p = rook_relevant_mask(a1);
    int n = __builtin_popcountll(p);
    for(int i = 0; i < 100; i++) {
        ull b = relevant_occupancy_mask(i, n, p);
        print_bitboard(b);
    }
    


    return 0;
}
