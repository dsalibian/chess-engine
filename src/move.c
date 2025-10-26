#include "move.h"
#include "bits.h"
#include "misc.h"
#include <stdio.h>

char mv_promochar(const move m) {
    static const char arr[] = {'n', 'b', 'r', 'q'};

    return arr[(MV_GETCODE(m) & ~MCODE_FPROMO)];
}

void dbgprint_mv(const move m, const u32* occupancy, const bool println) {
    assert(occupancy);

    const u32 code = MV_GETCODE(m);

    if(MCODE_ISCASTLE(code)) {
        printf(code == MCODE_CASTLE_K ? "oo" : "ooo");

        goto println;
    } 

    const u32 to   = MV_GETTO(m);
    const u32 from = MV_GETFROM(m);

    if(occupancy && OCCUPANCY_TYPE(occupancy[from]) != TYPE_PAWN)
        printf("%c", occupancy_char(occupancy[from])); 

    printf("%c%c%c%c%c",
            'a' + SQR_FILE(from),
            '1' + SQR_RANK(from),
            MCODE_ISCAP(code) ? 'x' : '-',
            'a' + SQR_FILE(to),
            '1' + SQR_RANK(to));

    if(MCODE_ISPROMO(code))
        printf("=%c", mv_promochar(m));
    else if(code == MCODE_EN_PASSANT)
        printf(" ep");

    println:;
    if(println)
        printf("\n");
}
