#include "perft.h"
#include "misc.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "movemake.h"
#include "position.h"
#include <stdio.h>
#include <string.h>


static u64 go(const struct atts_tbl* tbl, struct position* pos, const u32 depth) {
    struct move_stack ms = stack_new(); 
    genmoves_legal(
            tbl, 
            &ms, 
            pos->turn, 
            pos->castle_k[pos->turn], 
            pos->castle_q[pos->turn], 
            pos->en_passant_target, 
            pos->bb_all, 
            pos->bbs);

    if(depth < 2) {
        if(!depth)
            return 1;

        return ms.size;
    }

    u64 s = 0; 

    for(; ms.size;) {
        struct position cpy = *pos;

        move m = stack_pop(&ms);

        move_make(tbl, &cpy, m);

        s += go(tbl, &cpy, depth - 1);
    }

    return s;
}

void perft(const char* fen, u32 depth) {
    const struct atts_tbl tbl = gen_atts_tbl(); 

    struct position original;
    if(fen)
        pos_fen(&original, fen);
    else
        pos_startpos(&original);

    struct move_stack ms = stack_new();
    genmoves_legal(
            &tbl, 
            &ms, 
            original.turn, 
            original.castle_k[original.turn], 
            original.castle_q[original.turn], 
            original.en_passant_target, 
            original.bb_all, 
            original.bbs);

    u64 total = 0;

    for(; ms.size; ) {
        struct position pos = original;

        move m = stack_pop(&ms);

        uciprint_mv(m, false);

        move_make(&tbl, &pos, m);

        u64 s = go(&tbl, &pos, depth - 1);

        printf("  %lu\n", s);

        total += s;
    }

    printf("%lu\n", total);
}
