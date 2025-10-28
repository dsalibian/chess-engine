#include "perft.h"
#include "misc.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "movemake.h"
#include <stdio.h>

u64 go(const struct atts_tbl* tbl, struct position* pos, const u32 depth, const u32 u) {
    if(depth == u)
        return 1;

    u64 s = 0;

    struct move_stack ms = stack_new();
    stack_push_moves(&ms, tbl, pos);

    for(; ms.size; ) {
        struct position cpy = *pos;

        const move m = stack_pop(&ms);
        move_make(tbl, &cpy, m);

        if(!sqr_attd_opps(
                    tbl, 
                    TZCNT(cpy.bbs[!cpy.turn][IDX_KING]), 
                    cpy.bb_all,
                    cpy.bbs[cpy.turn],
                    !cpy.turn))
            s += go(tbl, &cpy, depth + 1, u);
    }

    return s;
}

void perft(const char* fen, u32 depth) {
    const struct atts_tbl tbl = gen_atts_tbl(); 

    struct position original;
    if(fen)
        parse_fen(&original, fen);
    else
        startpos(&original);

    //move_make(&tbl, &original, MV_ENCODE(MCODE_DBL_PUSH, F4, F2)); --depth;
    //move_make(&tbl, &original, MV_ENCODE(MCODE_DBL_PUSH, H5, H7)); --depth;
    //move_make(&tbl, &original, MV_ENCODE(MCODE_QUIET, F2, E1)); --depth;
    //move_make(&tbl, &original, MV_ENCODE(MCODE_QUIET, H4, H5)); --depth;

    //dbgprint_pos(&original);


    struct move_stack ms = stack_new();
    stack_push_moves(&ms, &tbl, &original);

    struct position pos = original;

    u64 total = 0;

    for(; ms.size; ) {
        pos = original;

        move m = stack_pop(&ms);
        move_make(&tbl, &pos, m);

        if(!sqr_attd_opps(
                    &tbl, 
                    TZCNT(pos.bbs[!pos.turn][IDX_KING]), 
                    pos.bb_all,
                    pos.bbs[pos.turn],
                    !pos.turn)) 
        {
            uciprint_mv(m, false); 

            u64 t = go(&tbl, &pos, 1, depth);

            printf("  %lu\n", t);

            total += t;
        }
    }

    printf("%lu\n", total);
}
