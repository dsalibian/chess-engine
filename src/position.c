#include "position.h"
#include "types.h"
#include "misc.h"
#include "bits.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static u8 char_type(const char c) {
    const char t = (char)tolower(c);

    return 
        t == 'p' ? TYPE_PAWN  :
        t == 'n' ? TYPE_NIGHT :
        t == 'b' ? TYPE_BISHP :
        t == 'r' ? TYPE_ROOK  :
        t == 'q' ? TYPE_QUEEN :
        t == 'k' ? TYPE_KING  : TYPE_EMPTY;
}

void pos_fen(struct position* pos, const char* fen) {
    bitboard* const bbs_b = pos->bbs[TURN_B];
    bitboard* const bbs_w = pos->bbs[TURN_W];

    // set occupancy empty
    for(u32 s = 0; s < 64; ++s)
        pos->board[s] = TYPE_EMPTY;

    // set bitboards empty
    memset(pos->bbs, 0, 2 * 7 * sizeof(bitboard));

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

        u8 type         = char_type(c);
        i32 sqr         = 8 * rank + file;
        bitboard sqr_bb = SQR_BB(sqr);

        pos->board[sqr] = type;
        pos->bb_all    |= sqr_bb;  

        if(isupper(c)) {
            bbs_w[type]    |= sqr_bb;
            bbs_w[IDX_ALL] |= sqr_bb;
        } else {
            bbs_b[type]    |= sqr_bb;
            bbs_b[IDX_ALL] |= sqr_bb;
        }

        ++file;
    }

    for(; *fen == ' '; ++fen);

    // turn
    pos->turn = *fen == 'w';
    for(; *(++fen) == ' ';);

    // castling
    pos->castle_k[0] = pos->castle_k[1] = pos->castle_q[0] = pos->castle_q[1] = false;
    for(; *fen != ' '; ++fen) {
        if(*fen == 'K') pos->castle_k[TURN_W] = true;
        if(*fen == 'k') pos->castle_k[TURN_B] = true;
        if(*fen == 'Q') pos->castle_q[TURN_W] = true;
        if(*fen == 'q') pos->castle_q[TURN_B] = true;
    }
    for(; *(++fen) == ' ';);

    // set en-passant target
    if(*fen == '-')
        pos->en_passant_target = 0;
    else {
        pos->en_passant_target = (u8)((*(fen + 1) - '0' - 1) * 8 + (*fen - 'a'));

        assert(SQR_RANK(pos->en_passant_target) == (pos->turn ? 5 : 2));
        ++fen;
    }
    for(; *(++fen) == ' ';);

    // half-move clock
    char* e;
    u64 t = strtoul(fen, &e, 10);

    fen = e;
    pos->hmoves = (u32)t;

    // full-move clock
    t = strtoul(fen, NULL, 10);

    pos->fmoves = (u32)t;
}

void pos_startpos(struct position* pos) {
    pos_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void pos_print(const struct position* pos) {
    board_print(pos->board, pos->bbs[TURN_W][IDX_ALL]);

    printf("turn:   %c\n", pos->turn == TURN_W ? 'w' : 'b');
    printf("hmoves: %d\n", pos->hmoves);
    printf("fmoves: %d\n", pos->fmoves);

    if(pos->en_passant_target)
        printf("ep:     %c%c\n", 'a' + SQR_FILE(pos->en_passant_target), '1' + SQR_RANK(pos->en_passant_target));
    else
        printf("ep:     none\n");

    printf("oo_w:   %c\n",   pos->castle_k[TURN_W] ? 'y' : 'n');
    printf("ooo_w:  %c\n",   pos->castle_q[TURN_W] ? 'y' : 'n');
    printf("oo_b:   %c\n",   pos->castle_k[TURN_B] ? 'y' : 'n');
    printf("ooo_b:  %c\n\n", pos->castle_q[TURN_B] ? 'y' : 'n');
}
