#include "position.h"
#include "types.h"
#include "misc.h"
#include "bits.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

u32 char_type(const char c) {
    const char t = (char)tolower(c);

    return 
        t == 'p' ? TYPE_PAWN  :
        t == 'n' ? TYPE_NIGHT :
        t == 'b' ? TYPE_BISHP :
        t == 'r' ? TYPE_ROOK  :
        t == 'q' ? TYPE_QUEEN :
        t == 'k' ? TYPE_KING  : TYPE_NONE;
}

u32 char_occupancy(const char c) {
    return char_type(c) + (islower(c) ? OCCUPANCY_B_PAWN : 0);
}

void parse_fen(struct position* pos, const char* fen) {
    bitboard* const bbs_b = pos->bbs[TURN_B];
    bitboard* const bbs_w = pos->bbs[TURN_W];

    // set occupancy empty
    for(u32 s = 0; s < 64; ++s)
        pos->occupancy[s] = OCCUPANCY_EMPTY;

    // set bitboards empty
    memset(pos->bbs, 0, 2 * 7 * sizeof(bitboard));
    pos->bb_all = 0;


    // occupancy and bitboards
    for(i32 rank = 7, file = 0; *fen != ' '; ++fen) {
        char c = *fen;
        
        if(c == '/') {
            --rank;
            file = 0;

            continue;
        }

        if(isdigit(c)) {
            file += c - '0';

            continue;
        }

        u32 occupancy   = char_occupancy(c);
        u32 type        = OCCUPANCY_TYPE(occupancy);
        i32 sqr         = 8 * rank + file;
        bitboard sqr_bb = SQR_BB(sqr);

        assert(sqr < 64);
        assert(occupancy < OCCUPANCY_EMPTY);
        assert( !(pos->bb_all & sqr_bb) );

        pos->occupancy[sqr] = occupancy;
        pos->bb_all |= sqr_bb;  

        (OCCUPANCY_ISWHITE(occupancy) ? bbs_w : bbs_b)[IDX_ALL] |= sqr_bb;
        (OCCUPANCY_ISWHITE(occupancy) ? bbs_w : bbs_b)[type]    |= sqr_bb;

        pos->occupancy[sqr] = occupancy;

        ++file;
    }

    ++fen;

    // turn
    pos->turn = *(fen) == 'w';
    fen += 2;

    // castling
    pos->oo[0] = pos->oo[1] = pos->ooo[0] = pos->ooo[1] = false;
    for(; *fen != ' '; ++fen) {
        if(*fen == 'K') pos->oo [TURN_W] = true;
        if(*fen == 'k') pos->oo [TURN_B] = true;
        if(*fen == 'Q') pos->ooo[TURN_W] = true;
        if(*fen == 'q') pos->ooo[TURN_B] = true;
    }
    ++fen;

    // set en-passant target
    if(*fen == '-')
        pos->en_passant_target = 0;
    else {
        pos->en_passant_target = (u32)((*(fen + 1) - '0' - 1) * 8 + *fen - 'a');
        assert(SQR_RANK(pos->en_passant_target) == (pos->turn == TURN_W ? 5 : 2));
        ++fen;
    }
    fen += 2;

    // half-move clock
    char* e;
    u64 t = strtoul(fen, &e, 10);
    assert(t < 2048);

    fen = e;
    pos->hmoves = (u32)t;

    // full-move clock
    t = strtoul(fen, NULL, 10);
    assert(t < 1024);

    pos->fmoves = (u32)t;
}

void startpos(struct position* pos) {
    parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void dbgprint_pos(const struct position* pos) {
    print_occupancy(pos->occupancy);

    printf("turn:   %c\n", pos->turn == TURN_W ? 'w' : 'b');
    printf("hmoves: %d\n", pos->hmoves);
    printf("fmoves: %d\n", pos->fmoves);

    if(pos->en_passant_target)
        printf("ep:     %c%c\n", 'a' + SQR_FILE(pos->en_passant_target), '1' + SQR_RANK(pos->en_passant_target));
    else
        printf("ep:     none\n");

    printf("oo_w:   %c\n",   pos->oo [TURN_W] ? 'y' : 'n');
    printf("ooo_w:  %c\n",   pos->ooo[TURN_W] ? 'y' : 'n');
    printf("oo_b:   %c\n",   pos->oo [TURN_B] ? 'y' : 'n');
    printf("ooo_b:  %c\n\n", pos->ooo[TURN_B] ? 'y' : 'n');
}
