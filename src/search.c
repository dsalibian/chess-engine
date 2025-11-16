#include "search.h"
#include "eval.h"
#include "bits.h"
#include "movemake.h"
#include <stdio.h>

extern bitboard sqr_attrs_bb2(const struct atts_tbl*, const bitboard, const bitboard[2][7], const u32, const bool);

i32 qss(const struct atts_tbl* tbl, const struct position* pos, i32 a, i32 b) {
    struct move_stack caps = stack_new();
    genmoves_legal(
            tbl,
            &caps,
            pos->turn,
            pos->castle_k[pos->turn],
            pos->castle_q[pos->turn],
            pos->en_passant_target,
            pos->bb_all,
            pos->bbs,
            GENTYPE_CAPS);     

    i32 v = eval(tbl, pos);

    if(v >= b)
        return v;

    if(v > a)
        a = v;


    for(; caps.size; ) {
        struct position cpy = *pos;

        move m = stack_pop(&caps);
        move_make(tbl, &cpy, m);

        i32 w = -qss(tbl, &cpy, -b, -a);

        if(w > b)
            return w;

        if(w > v)
            v = w;

        if(w > a)
            a = w;
    }

    return v;
}

i32 nega(const struct atts_tbl* tbl, const struct position* pos, u32 depth, i32 a, i32 b, bool root) {
    struct move_stack ms = stack_new();
    genmoves_legal(
            tbl,
            &ms,
            pos->turn,
            pos->castle_k[pos->turn],
            pos->castle_q[pos->turn],
            pos->en_passant_target,
            pos->bb_all,
            pos->bbs,
            GENTYPE_QUIET);     
    genmoves_legal(
            tbl,
            &ms,
            pos->turn,
            pos->castle_k[pos->turn],
            pos->castle_q[pos->turn],
            pos->en_passant_target,
            pos->bb_all,
            pos->bbs,
            GENTYPE_CAPS);     

    if(!ms.size) {
        const bool mate = 
            sqr_attrs_bb2(tbl, pos->bb_all, pos->bbs, TZCNT(pos->bbs[pos->turn][IDX_KING]), !pos->turn);

        return mate ? -EVAL_MATE : 0;
    } 

    if(!depth) {
        const i32 v = qss(tbl, pos, a, b);

        return v;
    }

    i32 best = -999999;
    move bestm = ms.ls[0];

    for(; ms.size; ) {
        struct position cpy = *pos;

        u16 m = stack_pop(&ms);
        move_make(tbl, &cpy, m);
            
        i32 v = -nega(tbl, &cpy, depth - 1, -b, -a, false);

        if(v > best) {
            best = v;
            bestm = m;

            if(v > a)
                a = v;

            if(root) {
                uciprint_mv(m, false);
                printf(" %d\n", v);
            }
        }

        if(v > b)
            return best;
    }

    return root ? bestm : best;
}
