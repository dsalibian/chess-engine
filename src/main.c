#include "misc.h"
#include "types.h"
#include "bits.h"
#include "movegen.h"
#include "magic.h"
#include "move.h"
#include "position.h"
#include "movemake.h"
#include "perft.h"
#include "eval.h"
#include "search.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//void test() {
//    const struct atts_tbl tbl = gen_atts_tbl();
//
//    struct position p;
//    pos_startpos(&p);
//
//    for(;;) {
//        move m = (u16)nega(&tbl, &p, 6, -999999, 999999, true);
//        move_make(&tbl, &p, m);
//
//        pos_print(&p); 
//        struct move_stack ms = stack_new();
//        genmoves_legal(
//                &tbl, 
//                &ms, 
//                p.turn, 
//                p.castle_k[p.turn], 
//                p.castle_q[p.turn], 
//                p.en_passant_target, 
//                p.bb_all, 
//                p.bbs,
//                GENTYPE_ALL);
//        stack_print(&ms);
//
//        u32 s;
//        scanf("%u", &s);
//        for(;getchar() != '\n' && !feof(stdin);){};
//
//        move_make(&tbl, &p, ms.ls[s]);
//    }
//}


extern u64 nodes;

int main(int argc, char** argv) {

    perft(NULL, 7);

    //const struct atts_tbl tbl = gen_atts_tbl();

    //struct position p;
    ////pos_startpos(&p);
    //pos_fen(&p, "rnbq1rk1/pppp1ppp/5n2/4p3/1bP5/2N3P1/PP1PPPBP/R1BQK1NR w KQ - 5 5");

    //////pos_fen(&p, "rnbqkbnr/pppp1p1p/6p1/4p2Q/4P3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 3");

    //uciprint_mv((u16)nega(&tbl, &p, 7, -EVAL_MATE, EVAL_MATE, true), true);
    //printf("%lu\n", nodes);

    //struct ttbl ztbl = zobrist_new(1024);

    //printf("%lx\n", gethash(&ztbl, p.bbs, p.turn, p.castle_k, p.castle_q, p.en_passant_target));


    return 0;
}
