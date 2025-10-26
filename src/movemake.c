#include "movemake.h"
#include "bits.h"
#include "misc.h"
#include "types.h"
#include <string.h>

void move_make(struct position* pos, const move m) {
    u32* occupancy = pos->occupancy;
    bool* turn     = &pos->turn;

    bitboard* us   = pos->bbs[*turn];
    bitboard* opps = pos->bbs[!*turn];
    bitboard* all  = &pos->bb_all;
    
    const u32 code = MV_GETCODE(m);
    const u32 from = MV_GETFROM(m);
    const u32 to   = MV_GETTO(m);

    const u32 moved_type = OCCUPANCY_TYPE(occupancy[from]);
    const u32 cap_type   = OCCUPANCY_TYPE(occupancy[to]);

    // update bitboards
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

            bitboard rk_src  = SQR_BB(k ? H1 : A1);
            bitboard rk_dest = SQR_BB(k ? F1 : D1);

            if(turn == TURN_B) {
                rk_src  <<= 56;
                rk_dest <<= 56;
            }

            assert(us[IDX_ROOK] & rk_src);

            const bitboard rk_move = rk_src | rk_dest;

            us[IDX_ROOK] ^= rk_move;
            us[IDX_ALL]  ^= rk_move;
        }

        if(MCODE_ISPROMO(code)) {
            const u32 promo_type = MCODE_PROMOTYPE(code);

            assert(us[IDX_PAWN] & to_bb);
            assert(!(us[promo_type] & to_bb));

            us[IDX_PAWN]   ^= to_bb;
            us[promo_type] ^= to_bb;
        }

        if(code == MCODE_EN_PASSANT) {
            const bitboard cap = *turn ? to >> 7 : to << 8;

            assert(opps[IDX_PAWN] & cap);

            opps[IDX_PAWN] ^= cap;
            opps[IDX_ALL]  ^= cap;
        } else if(MCODE_ISCAP(code)) {
            assert(cap_type < TYPE_NONE);
            assert(opps[IDX_ALL] & to_bb);

            opps[cap_type] ^= to_bb;
            opps[IDX_ALL]  ^= to_bb;
        }

        *all = us[IDX_ALL] | opps[IDX_ALL];
    }

    // update occupancy
    {
        occupancy[to]   = occupancy[from];
        occupancy[from] = OCCUPANCY_EMPTY;

        if(MCODE_ISCASTLE(code)) {
            const bool k = code == MCODE_CASTLE_K;

            bitboard rk_src  = k ? H1 : A1;
            bitboard rk_dest = k ? F1 : D1;

            if(*turn == TURN_B) {
                rk_src  += 56;
                rk_dest += 56;
            }
            
            assert(occupancy[rk_src] == (*turn ? OCCUPANCY_W_ROOK : OCCUPANCY_B_ROOK));

            occupancy[rk_dest] = occupancy[rk_src];
            occupancy[rk_src]  = OCCUPANCY_EMPTY;
        }

        if(MCODE_ISPROMO(code)) {
            assert(occupancy[to] == (*turn ? OCCUPANCY_W_PAWN : OCCUPANCY_B_PAWN));

            occupancy[to] = MCODE_PROMOTYPE(code);
        }

        if(code == MCODE_EN_PASSANT) {
            const u32 cap = *turn ? to - 8 : to + 8;

            assert(SQR_RANK(cap) == (*turn ? 4 : 3));

            occupancy[cap] = OCCUPANCY_EMPTY;
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
        
        pos->en_passant_target = code == MCODE_DBL_PUSH ? (*turn ? to - 8 : to + 8) : 0;

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


#include <stdio.h>
void move_unmake(struct position* pos, const move m, const struct umake_dat* umake) {
    u32* occupancy = pos->occupancy;
    bool* turn     = &pos->turn;

    bitboard* us   = pos->bbs[!*turn];
    bitboard* opps = pos->bbs[*turn];
    bitboard* all  = &pos->bb_all;
    
    const u32 code = MV_GETCODE(m);
    const u32 from = MV_GETFROM(m);
    const u32 to   = MV_GETTO(m);

    const u32 moved_type    = OCCUPANCY_TYPE(occupancy[to]);
    const u32 cap_type      = umake->captured_type;
    const u32 cap_occupancy = TYPE_OCCUPANCY(cap_type, *turn);

    // update bitboards
    {
        const bitboard from_bb = SQR_BB(from);
        const bitboard to_bb   = SQR_BB(to);
        const bitboard move_bb = from_bb | to_bb;

        us[moved_type] ^= move_bb;
        us[IDX_ALL]    ^= move_bb;

        if(MCODE_ISCASTLE(code)) {
            const bool k = code == MCODE_CASTLE_K;

            bitboard rk_src  = SQR_BB(k ? H1 : A1);
            bitboard rk_dest = SQR_BB(k ? F1 : D1);

            if(*turn == TURN_B) {
                rk_src  <<= 56;
                rk_dest <<= 56;
            }

            const bitboard rk_move = rk_src | rk_dest;

            us[IDX_ROOK] ^= rk_move;
            us[IDX_ALL]  ^= rk_move;
        }

        if(MCODE_ISPROMO(code)) {
            const u32 promo_type = MCODE_PROMOTYPE(code);

            us[IDX_PAWN]   ^= to_bb;
            us[promo_type] ^= to_bb;
        }

        if(code == MCODE_EN_PASSANT) {
            const bitboard cap = !*turn ? to >> 7 : to << 8;

            opps[IDX_PAWN] ^= cap;
            opps[IDX_ALL]  ^= cap;
        } else if(MCODE_ISCAP(code)) {
            opps[cap_type] ^= to_bb;
            opps[IDX_ALL]  ^= to_bb;
        }
        
        *all = us[IDX_ALL] | opps[IDX_ALL];
    }


    // update occupancy board
    {
        occupancy[from] = occupancy[to];
        occupancy[to]   = cap_occupancy;

        if(MCODE_ISCASTLE(code)) {
            const bool k = code == MCODE_CASTLE_K;

            bitboard rk_src  = k ? H1 : A1;
            bitboard rk_dest = k ? F1 : D1;

            if((!*turn) == TURN_B) {
                rk_src  += 56;
                rk_dest += 56;
            }
            
            occupancy[rk_dest] = OCCUPANCY_EMPTY;
            occupancy[rk_src]  = TYPE_OCCUPANCY(TYPE_ROOK, !*turn);
        }

        if(MCODE_ISPROMO(code)) {
            occupancy[to]   = OCCUPANCY_EMPTY;
            occupancy[from] = TYPE_OCCUPANCY(TYPE_PAWN, !*turn);
        }
        
        if(code == MCODE_EN_PASSANT) {
            const u32 cap = !*turn ? to - 8 : to + 8;

            occupancy[cap] = TYPE_OCCUPANCY(TYPE_PAWN, *turn);
        }
    }

    // update position data
    {
        if(*turn)
            --(pos->fmoves);

        *turn = !*turn;

        pos->en_passant_target = umake->en_passant_target;
        pos->hmoves            = umake->hmoves;

        memcpy(pos->castle_k, umake->castle_k, 2);
        memcpy(pos->castle_q, umake->castle_q, 2);
    }
}

struct umake_dat set_umakedat(const struct position* pos, const move m) {
    struct umake_dat u = (struct umake_dat) {
        .hmoves = pos->hmoves, 
        .en_passant_target = pos->en_passant_target,
        .captured_type = OCCUPANCY_TYPE(pos->occupancy[MV_GETTO(m)])
    };

    memcpy(u.castle_k, pos->castle_k, 2);
    memcpy(u.castle_q, pos->castle_q, 2);

    return u;
}
