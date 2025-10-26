#include "misc.h"
#include "bits.h"
#include <stdio.h>


char occupancy_char(const u32 occupancy) {
    static const char arr[] = 
    {
        'P', 'N', 'B', 'R', 'Q', 'K',
        'p', 'n', 'b', 'r', 'q', 'k', '.'
    };
    assert(occupancy <= OCCUPANCY_EMPTY);

    return arr[occupancy];
}

void print_occupancy(const u32* occupancy) {
    assert(occupancy);

    for(i32 r = 7; r > -1; --r) {
        printf(" %d   ", r + 1);

        for(i32 f = 0; f < 8; ++f)
            printf("%c ", occupancy_char(occupancy[8 * r + f]));

        printf("\n");
    }

    printf("\n     a b c d e f g h\n\n");
}
