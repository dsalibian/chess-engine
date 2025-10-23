#include "bits.h"
#include <stdio.h>

void print_bb(const bitboard bb) {
    for(i32 r = 7; r > -1; --r) {
        printf(" %d   ", r + 1);

        for(i32 f = 0; f < 8; ++f)
            printf("%c ", GET_BIT(bb, 8 * r + f) ? '1' : '.');

        printf("\n");
    }
    printf("\n     a b c d e f g h\n\n");
}
