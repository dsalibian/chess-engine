#include "misc.h"

void print_bitboard(ull board) {
    for(int i = 7; i > -1; i--) {
        cout << i + 1 << "   ";
        for(int j = 0; j < 8; j++) {
            int k = 8 * i + j;
            cout << ( get_bit(board, k) ? "1" : ".") << " ";
        }
        cout << '\n';
    }
    cout << "\n    a b c d e f g h\n\n\n";
}

bool get_bit(ull n, int pos) {
    return ( n >> pos ) & 1;
}

void set_bit(ull& n, int pos, bool bit) {
    get_bit(n, pos) == bit ? 0 : n ^= ( 1ULL << pos );
}

// return index of lsb and pop lsb
int pop_lsb(ull& n) {
    int i = __builtin_ctzll(n);
    n &= ~( 1ULL << i );
    return i; 
}

ull BB(int n) {
    return 1ULL << n;
}

