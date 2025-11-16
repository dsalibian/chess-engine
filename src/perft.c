#include "perft.h"
#include "misc.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "movemake.h"
#include "position.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static u64 time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (u64)(tv.tv_sec) * 1000 + (u64)(tv.tv_usec / 1000);
}

static u64 _perft(const struct atts_tbl* tbl, const struct position* pos, const u32 depth, const bool root) {
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
            GENTYPE_ALL);

    if(depth < 2 && !root) 
        return depth ? ms.size : 1;

    u64 s = 0; 

    for(; ms.size;) {
        struct position cpy = *pos;

        move m = stack_pop(&ms);

        move_make(tbl, &cpy, m);

        u64 t = _perft(tbl, &cpy, depth - 1, false);
        s += t;

        if(root) {
            uciprint_mv(m, false);
            printf("  %lu\n", t);
        }
    }

    return s;

}

void perft(const char* fen, const u32 depth) {
    const struct atts_tbl tbl = gen_atts_tbl(); 

    struct position pos;
    if(fen)
        pos_fen(&pos, fen);
    else
        pos_startpos(&pos);

    u64 t = time_ms();

    const u64 c = _perft(&tbl, &pos, depth, true);

    t = time_ms() - t;

    printf("\n%lu nodes\n\n%.3f s\n%ld nps\n", c, (double)t / 1000., c * 1000 / (t + !t));
}
