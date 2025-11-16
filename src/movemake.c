#include "movemake.h"
#include "bits.h"
#include "misc.h"
#include "move.h"
#include "types.h"
#include <string.h>
#include <stdio.h>


void move_make(const struct atts_tbl* tbl, struct position* pos, const move m) {
    u8* board      = pos->board;
    bool* turn     = &pos->turn;

    bitboard* us   = pos->bbs[*turn];
    bitboard* opps = pos->bbs[!*turn];
    bitboard* all  = &pos->bb_all;
    
    const u32 code = MV_GETCODE(m);
    const u32 from = MV_GETFROM(m);
    const u32 to   = MV_GETTO(m);

    const u32 moved_type = board[from];
    const u32 cap_type   = board[to];
    
    // bitboards
    {
        const bitboard from_bb = SQR_BB(from);
        const bitboard to_bb   = SQR_BB(to);
        const bitboard move_bb = from_bb | to_bb;

        assert(us[IDX_ALL] & from_bb);
        assert(!(us[IDX_ALL] & to_bb));

        us[moved_type] ^= move_bb;
        us[IDX_ALL]    ^= move_bb;

        if(MCODE_ISCASTLE(code)) {
            const bool k = code == MCODE_CASTLE_K;

            const bitboard rbb = tbl->castle_r_bb[*turn][!k];

            assert(us[IDX_ROOK] & SQR_BB(tbl->castle_r_from[*turn][!k]));

            us[IDX_ROOK] ^= rbb;
            us[IDX_ALL]  ^= rbb;
        }

        else if(MCODE_ISPROMO(code)) {
            const u32 promo_type = MCODE_PROMOTYPE(code);

            assert(us[IDX_PAWN] & to_bb);
            assert(!(us[promo_type] & to_bb));

            us[IDX_PAWN]   ^= to_bb;
            us[promo_type] ^= to_bb;
        }

        if(MCODE_ISCAP(code)) {
            if(code == MCODE_EN_PASSANT) {
                const bitboard cap = tbl->ppush[!*turn][to];

                assert(opps[IDX_PAWN] & cap);

                opps[IDX_PAWN] ^= cap;
                opps[IDX_ALL]  ^= cap;
            } else {
                assert(cap_type < TYPE_EMPTY);
                assert(opps[IDX_ALL] & to_bb);

                opps[cap_type] ^= to_bb;
                opps[IDX_ALL]  ^= to_bb;
            }
        }

        *all = us[IDX_ALL] | opps[IDX_ALL];
    }

    // occupancy 
    {
        board[to]   = board[from];
        board[from] = TYPE_EMPTY;

        if(MCODE_ISCASTLE(code)) {
            const bool k = code == MCODE_CASTLE_K;

            const u32 rk_src  = tbl->castle_r_from[*turn][!k];
            const u32 rk_dest = tbl->castle_r_to  [*turn][!k];
            
            assert(board[rk_src] == TYPE_ROOK);

            board[rk_dest] = board[rk_src];
            board[rk_src]  = TYPE_EMPTY;
        }

        else if(MCODE_ISPROMO(code)) {
            assert(board[to] == TYPE_PAWN);

            board[to] = (u8)MCODE_PROMOTYPE(code);
        }

        else if(code == MCODE_EN_PASSANT) {
            const bitboard cap = *turn ? to - 8 : to + 8;

            assert(SQR_RANK(cap) == (*turn ? 4 : 3));

            board[cap] = TYPE_EMPTY;
        }
    }

    // update position data
    {
        if(!*turn)
            ++(pos->fmoves);

        if(MCODE_ISCAP(code) || moved_type == TYPE_PAWN)
            pos->hmoves = 0;
        else 
            ++(pos->hmoves);
        
        pos->en_passant_target = code == MCODE_DBL_PUSH ? (u8)(*turn ? to - 8 : to + 8) : 0;

        if(MCODE_ISCASTLE(code) || moved_type == TYPE_KING)
            pos->castle_k[*turn] = pos->castle_q[*turn] = false;

        if(moved_type == TYPE_ROOK) {
            if(from == H1) pos->castle_k[TURN_W] = false;
            if(from == H8) pos->castle_k[TURN_B] = false;
            if(from == A1) pos->castle_q[TURN_W] = false;
            if(from == A8) pos->castle_q[TURN_B] = false;
        }

        if(cap_type == TYPE_ROOK) {
            if(to == H1) pos->castle_k[TURN_W] = false;
            if(to == H8) pos->castle_k[TURN_B] = false;
            if(to == A1) pos->castle_q[TURN_W] = false;
            if(to == A8) pos->castle_q[TURN_B] = false;
        }

        *turn = !*turn;
    }
}
