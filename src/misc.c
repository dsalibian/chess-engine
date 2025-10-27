#include "misc.h"
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


bool sqr_attd_opps(
        const struct atts_tbl* tbl, 
        const u32 sqr, 
        const bitboard all, 
        const bitboard* opps, 
        const bool turn)
{
    const bitboard batts = bmoves_bb(tbl, sqr, all, 0);
    const bitboard ratts = rmoves_bb(tbl, sqr, all, 0);  
    const bitboard qatts = batts | ratts;

    return 
        (opps[IDX_PAWN]  & tbl->patts[turn][sqr]) ||
        (opps[IDX_NIGHT] & tbl->natts[sqr]) ||
        (opps[IDX_KING]  & tbl->katts[sqr]) ||
        (opps[IDX_BISHP] & batts) || 
        (opps[IDX_ROOK]  & ratts) || 
        (opps[IDX_QUEEN] & qatts); 
}
