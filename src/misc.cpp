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

int get_bit(ull n, int pos) {
    return ( n >> pos ) & 1;
}