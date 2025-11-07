#include "misc.h"
#include "bits.h"
#include <stdio.h>
#include <ctype.h>

char occupancy_char(const u32 occupancy) {
    static const char arr[] = 
    {
        'P', 'N', 'B', 'R', 'Q', 'K',
        'p', 'n', 'b', 'r', 'q', 'k', '.'
    };

    return arr[occupancy];
}

char type_char(const u8 type) {
    static const char arr[] = {'p', 'n', 'b', 'r', 'q', 'k', '.'};

    return arr[type];
}

void board_print(const u8* board, const bitboard w_bb) {
    assert(board);

    for(i32 r = 7; r > -1; --r) {
        printf(" %d   ", r + 1);

        for(i32 f = 0; f < 8; ++f)
            printf("%c ", occupancy_char(BOARD_OCCUPANCY(board, w_bb, 8 * r + f)));

        printf("\n");
    }

    printf("\n     a b c d e f g h\n\n");
}
