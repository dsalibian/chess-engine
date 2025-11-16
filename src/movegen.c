#include "movegen.h"
#include "bits.h"
#include "move.h"
#include "position.h"
#include "misc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NORTH_EAST,
    NORTH_WEST,
    SOUTH_EAST,
    SOUTH_WEST,
};

static bitboard gen_ray(const u32 sqr, const bitboard blk, u32 dir) {
    const i8 df[8] = { 0,  1,  0, -1,  1, -1,  1, -1};
    const i8 dr[8] = { 1,  0, -1,  0,  1,  1, -1, -1};

    bitboard bb = 0;

    for(i32 f = SQR_FILE(sqr), r = SQR_RANK(sqr); (u32)(f += df[dir]) < 8 && (u32)(r += dr[dir]) < 8; ) {
        bitboard s = SQR_BB(8 * r + f);
        bb |= s;

        if(s & blk)
            break;
    }

    return bb;
}

bitboard gen_satts(const u32 sqr, const bool bsp, const bitboard blk) {
    bitboard atts = 0;

    for(u32 dir = bsp ? NORTH_EAST : NORTH; dir <= (bsp ? SOUTH_WEST : WEST); ++dir)
        atts |= gen_ray(sqr, blk, dir);
    
    return atts; 
}

bitboard gen_ratts(const u32 sqr, const bitboard blk) {
    return gen_satts(sqr, false, blk);
}

bitboard gen_batts(const u32 sqr, const bitboard blk) {
    return gen_satts(sqr, true, blk);
}

bitboard gen_patts(const u32 sqr, const bool w) {
    const bitboard sqr_bb = SQR_BB(sqr);
    const u32 f           = SQR_FILE(sqr);
    bitboard atts         = 0;

    if(f > 0) atts |= w ? sqr_bb << 7 : sqr_bb >> 9;
    if(f < 7) atts |= w ? sqr_bb << 9 : sqr_bb >> 7;

    return atts;
}

bitboard gen_natts(const u32 sqr) {
    const bitboard sqr_bb = SQR_BB(sqr);
    const u32 f           = SQR_FILE(sqr);
    bitboard atts         = 0;

    if(f > 0) atts |= (sqr_bb << 15) | (sqr_bb >> 17);
    if(f > 1) atts |= (sqr_bb << 6 ) | (sqr_bb >> 10);
    if(f < 6) atts |= (sqr_bb << 10) | (sqr_bb >> 6 );
    if(f < 7) atts |= (sqr_bb << 17) | (sqr_bb >> 15);

    return atts;
}

bitboard gen_katts(const u32 sqr) {
    const bitboard sqr_bb = SQR_BB(sqr);
    const u32 f           = SQR_FILE(sqr);
    bitboard atts         = (sqr_bb << 8) | (sqr_bb >> 8);

    if(f < 7) atts |= (sqr_bb << 1) | (sqr_bb << 9) | (sqr_bb >> 7);
    if(f > 0) atts |= (sqr_bb >> 1) | (sqr_bb >> 9) | (sqr_bb << 7);

    return atts;
}

bitboard gen_rmask(const u32 sqr, const bool bsp) {
    bitboard m = gen_satts(sqr, bsp, 0);

    if(bsp)
        return m & ~(FILE_AH | RANK_18);

    const u32 f = SQR_FILE(sqr);
    const u32 r = SQR_RANK(sqr);

    if(f > 0) m &= ~FILE_A;
    if(f < 7) m &= ~FILE_H;

    if(r > 0) m &= ~RANK_1;
    if(r < 7) m &= ~RANK_8;

    return m;
}

struct atts_tbl gen_atts_tbl() {
    struct atts_tbl tbl;

    u64 bsp_s = 0x6f67ea16a95f0393ull;
    u64 rk_s  = 0x9671405820c301a7ull;

    for(u32 s = 0; s < 64; ++s) {
        tbl.bsp_magic[s]  = magic_make(s, true,  &bsp_s);        
        tbl.rk_magic[s]   = magic_make(s, false, &rk_s);        

        tbl.patts[TURN_W][s] = gen_patts(s, true);
        tbl.patts[TURN_B][s] = gen_patts(s, false);

        tbl.natts[s] = gen_natts(s);
        tbl.katts[s] = gen_katts(s);

        tbl.ppush[TURN_W][s] = SQR_BB(s) << 8;
        tbl.ppush[TURN_B][s] = SQR_BB(s) >> 8;

        if(SQR_RANK(s) == 1)
            tbl.dpush_check[TURN_W][s] = tbl.ppush[TURN_W][s] | (tbl.dpush[TURN_W][s] = SQR_BB(s) << 16);

        if(SQR_RANK(s) == 6)
            tbl.dpush_check[TURN_B][s] = tbl.ppush[TURN_B][s] | (tbl.dpush[TURN_B][s] = SQR_BB(s) >> 16);

    }

    // castle rook bitboards
    {
        tbl.castle_r_bb[TURN_W][0] = SQR_BB(H1) | SQR_BB(F1); // kingside
        tbl.castle_r_bb[TURN_W][1] = SQR_BB(A1) | SQR_BB(D1); // queenside

        tbl.castle_r_bb[TURN_B][0] = SQR_BB(H8) | SQR_BB(F8);
        tbl.castle_r_bb[TURN_B][1] = SQR_BB(A8) | SQR_BB(D8);
    }

    // castle rook squares
    {
        tbl.castle_r_to  [TURN_W][0] = F1; // kingside
        tbl.castle_r_to  [TURN_W][1] = D1; // queenside
        tbl.castle_r_from[TURN_W][0] = H1;
        tbl.castle_r_from[TURN_W][1] = A1;

        tbl.castle_r_to  [TURN_B][0] = F8;
        tbl.castle_r_to  [TURN_B][1] = D8;
        tbl.castle_r_from[TURN_B][0] = H8;
        tbl.castle_r_from[TURN_B][1] = A8;
    }

    // between and through bbs
    {
        memset(tbl.between_bb, 0, 64 * 64 * sizeof(bitboard));
        memset(tbl.through_bb, 0, 64 * 64 * sizeof(bitboard));

        static u8 bsp_dir[2][2] = {{SOUTH_WEST, NORTH_WEST}, {SOUTH_EAST, NORTH_EAST}};
        static u8 rk_dir [2][2] = {{WEST, SOUTH}, {EAST, NORTH}};

        for(u32 s0 = 0; s0 < 64; ++s0) {
            i32 f0 = SQR_FILE(s0);
            i32 r0 = SQR_RANK(s0);

            for(u32 s1 = 0; s1 < 64; ++s1) {
                if(s0 == s1)
                    continue;

                i32 f1 = SQR_FILE(s1);
                i32 r1 = SQR_RANK(s1);

                i32 df = f1 - f0;
                i32 dr = r1 - r0;

                if(abs(f0 - f1) == abs(r0 - r1)) {
                    tbl.between_bb[s0][s1] = gen_ray(s0, SQR_BB(s1), bsp_dir[df > 0][dr > 0]);

                    tbl.through_bb[s0][s1] = SQR_BB(s0) | 
                                                gen_ray(s0, 0, bsp_dir[df > 0][dr > 0]) |
                                                gen_ray(s0, 0, bsp_dir[df < 0][dr < 0]);
                }

                if(f0 == f1 || r0 == r1) {
                    tbl.between_bb[s0][s1] = gen_ray(s0, SQR_BB(s1), rk_dir[dr ? dr > 0 : df > 0][!!dr]);

                    tbl.through_bb[s0][s1] = SQR_BB(s0) | 
                                                gen_ray(s0, 0, rk_dir[dr ? dr > 0 : df > 0][!!dr]) |
                                                gen_ray(s0, 0, rk_dir[dr ? dr < 0 : df < 0][!!dr]);
                }
            }
        } 
    }
    
    return tbl;
}

bitboard bmoves_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard all, const bitboard us) {
    return magic_moves_bb(&tbl->bsp_magic[sqr], all, us);
}

bitboard rmoves_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard all, const bitboard us) {
    return magic_moves_bb(&tbl->rk_magic[sqr], all, us);
}

bitboard qmoves_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard all, const bitboard us) {
    return bmoves_bb(tbl, sqr, all, us) | rmoves_bb(tbl, sqr, all, us);
}

bitboard nmoves_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard us) {
    return tbl->natts[sqr] & ~us;
}

bitboard kmoves_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard us) {
    return tbl->katts[sqr] & ~us;
}

bitboard pmoves_push_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard all, const bool turn) {
    return tbl->ppush[turn][sqr] & ~all;
}

bitboard pmoves_dpush_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard all, const bool turn) {
    assert(SQR_RANK(sqr) == (turn ? 1 : 6));

    return !(all & tbl->dpush_check[turn][sqr]) ? tbl->dpush[turn][sqr] : 0;
}

bitboard pmoves_cap_bb(const struct atts_tbl* tbl, const u32 sqr, const bitboard opps, const bool turn) {
    return tbl->patts[turn][sqr] & opps;
}

bitboard pmoves_ep_bb(const struct atts_tbl* tbl, const u32 sqr, const u32 target, const bool turn) {
    assert(SQR_RANK(sqr) == (turn ? 4 : 3));

    return tbl->patts[turn][sqr] & SQR_BB(target);
}



bitboard sqr_attrs_bb2(
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard          bbs[2][7],
        const u32               sqr,
        const bool              turn)
{
    bitboard bb = 0;

    bb |= tbl->patts[!turn][sqr]      &  bbs[turn][IDX_PAWN];
    bb |= tbl->natts[sqr]             &  bbs[turn][IDX_NIGHT];
    bb |= tbl->katts[sqr]             &  bbs[turn][IDX_KING];
    bb |= bmoves_bb(tbl, sqr, all, 0) & (bbs[turn][IDX_BISHP] | bbs[turn][IDX_QUEEN]);
    bb |= rmoves_bb(tbl, sqr, all, 0) & (bbs[turn][IDX_ROOK]  | bbs[turn][IDX_QUEEN]);

    return bb;
}


static void push_movebb(
        struct move_stack*  ms,
        const u32           from,
        const u32           code,
        bitboard            moves)
{
    for(; moves; moves = POP_LSB(moves)) 
        stack_push(ms, MV_ENCODE(code, TZCNT(moves), from));
}

static void push_sn(
        struct move_stack*      ms,
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard          us,
        const bitboard          opps,
        const u32               ksqr,
        const bitboard          target,
        bitboard                us_pc,
        const bool              pinned,
        const u32               gentype,
        bitboard (*s)(const struct atts_tbl*, const u32, const bitboard, const bitboard),
        bitboard (*n)(const struct atts_tbl*, const u32, const bitboard))
{
    const bool quiet = gentype == GENTYPE_QUIET || gentype == GENTYPE_ALL;
    const bool caps  = gentype == GENTYPE_CAPS  || gentype == GENTYPE_ALL;

    assert((bool)s ^ (bool)n);
    assert(quiet || caps);

    for(; us_pc; us_pc = POP_LSB(us_pc)) {
        u32 from = TZCNT(us_pc);
        bitboard moves = s ? s(tbl, from, all, us) : n(tbl, from, us);

        if(target)
            moves &= target;

        if(pinned)
            moves &= tbl->through_bb[ksqr][from];

        if(quiet) push_movebb(ms, from, MCODE_QUIET,   moves & ~opps); 
        if(caps)  push_movebb(ms, from, MCODE_CAPTURE, moves &  opps); 
    }
}

static void push_eep(
        struct move_stack*      ms,
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard*         opps,
        const bitboard          us_pc,
        const u32               ksqr,
        const u32               ep_target,
        const bool              turn)
{
    static const bitboard ep_mask[2] = {RANK_4, RANK_5};

    bitboard bb = us_pc & ep_mask[turn] & tbl->patts[!turn][ep_target];

    for(; bb; bb = POP_LSB(bb)) {
        u32 from = TZCNT(bb);

        const bitboard opps_bsp = opps[IDX_BISHP] | opps[IDX_QUEEN];
        const bitboard opps_rk  = opps[IDX_ROOK]  | opps[IDX_QUEEN];

        const bitboard all2 = all ^ (ISO_LSB(bb) | SQR_BB(ep_target)| tbl->ppush[!turn][ep_target]);

        const bitboard bsp = bmoves_bb(tbl, ksqr, all2, 0);
        const bitboard rk  = rmoves_bb(tbl, ksqr, all2, 0);

        const bitboard checker = (bsp & opps_bsp) | (rk & opps_rk);

        if(!checker)
            stack_push(ms, MV_ENCODE(MCODE_EN_PASSANT, ep_target, from)); 
    }
}

static void push_p(
        struct move_stack*      ms, 
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard          opps,
        const u32               ksqr,
        const bitboard          target,
        bitboard                us_pc,
        const bool              pinned,
        const bool              turn,
        const u32               gentype)
{
    static const bitboard promo_mask[2] = {RANK_2, RANK_7};
    static const bitboard dpush_mask[2] = {RANK_7, RANK_2};

    const bool quiet = gentype == GENTYPE_QUIET || gentype == GENTYPE_ALL;
    const bool caps  = gentype == GENTYPE_CAPS  || gentype == GENTYPE_ALL;

    bitboard bb = us_pc & ~promo_mask[turn];
    for(; bb; bb = POP_LSB(bb)) {
        u32 from = TZCNT(bb);

        bitboard quietbb = pmoves_push_bb(tbl, from, all, turn);
        bitboard capsbb  = pmoves_cap_bb (tbl, from, opps, turn);

        if(target) {
            quietbb &= target; 
            capsbb  &= target;
        }

        if(pinned) {
            quietbb &= tbl->through_bb[ksqr][from];
            capsbb  &= tbl->through_bb[ksqr][from];
        }

        if(quiet) push_movebb(ms, from, MCODE_QUIET,   quietbb); 
        if(caps)  push_movebb(ms, from, MCODE_CAPTURE, capsbb); 
    } 

    bb = us_pc & promo_mask[turn];
    for(; bb; bb = POP_LSB(bb)) {
        u32 from = TZCNT(bb);

        bitboard quietbb = pmoves_push_bb(tbl, from, all, turn);
        bitboard capsbb  = pmoves_cap_bb (tbl, from, opps, turn);

        if(target) {
            quietbb &= target; 
            capsbb  &= target;
        }

        if(pinned) {
            quietbb &= tbl->through_bb[ksqr][from];
            capsbb  &= tbl->through_bb[ksqr][from];
        }

        if(quiet) 
            for(u32 i = 0; i < 4; ++i)
                push_movebb(ms, from, MCODE_FPROMO + i, quietbb); 

        if(caps) 
            for(u32 i = 0; i < 4; ++i)
                push_movebb(ms, from, (MCODE_FPROMO | MCODE_FCAP) + i, capsbb); 
    } 

    if(quiet) {
        bb = us_pc & dpush_mask[turn];
        for(; bb; bb = POP_LSB(bb)) {
            u32 from       = TZCNT(bb);
            bitboard moves = pmoves_dpush_bb(tbl, from, all, turn);

            if(target) 
                moves &= target; 

            if(pinned)
                moves &= tbl->through_bb[ksqr][from];

            push_movebb(ms, from, MCODE_DBL_PUSH, moves); 
        }
    }
}

static void push_k(
        struct move_stack*      ms,
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard          bbs[2][7],
        const bool              turn,
        const u32               ksqr,
        const u32               gentype)
{
    const bool quiet = gentype == GENTYPE_QUIET || gentype == GENTYPE_ALL;
    const bool caps  = gentype == GENTYPE_CAPS  || gentype == GENTYPE_ALL;

    bitboard moves = kmoves_bb(tbl, ksqr, bbs[turn][IDX_ALL]);
    bitboard legal = 0;

    for(; moves; moves = POP_LSB(moves))
        if(!sqr_attrs_bb2(tbl, all ^ bbs[turn][IDX_KING], bbs, TZCNT(moves), !turn))
            legal |= ISO_LSB(moves);

    if(quiet) push_movebb(ms, ksqr, MCODE_QUIET,   legal & ~bbs[!turn][IDX_ALL]);
    if(caps)  push_movebb(ms, ksqr, MCODE_CAPTURE, legal &  bbs[!turn][IDX_ALL]);
}


static void push_c(
        struct move_stack*      ms,
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard          bbs[2][7],
        const u32               ksqr,
        const bool              turn,
        const bool              q) 
{
    static const bitboard castle_path_bb[2][2] = {{0x60ull << 56, 0xeull << 56}, {0x60, 0xe}};
    static const u8 castle_trvl_sqr[2][2]      = {{F8, D8}, {F1, D1}};
    static const u8 castle_to_sqr[2][2]        = {{G8, C8}, {G1, C1}};

    const bool path_clear = !(all & castle_path_bb[turn][q]);

    if(!path_clear)
        return;

    const u32 trvl = castle_trvl_sqr[turn][q];
    const u32 to   = castle_to_sqr  [turn][q];

    const bool check = 
        sqr_attrs_bb2(tbl, all, bbs, trvl, !turn) ||
        sqr_attrs_bb2(tbl, all, bbs, to,   !turn);

    if(!check)
        stack_push(ms, MV_ENCODE(q ? MCODE_CASTLE_Q : MCODE_CASTLE_K, to, ksqr));
}

static bitboard get_pinmask(
        const struct atts_tbl*  tbl,
        const bitboard          all, 
        const bitboard          all_us,
        const bitboard*         opps,
        const u32               ksqr)
{
    const bitboard potentially_pinned = qmoves_bb(tbl, ksqr, all, 0) & all_us; 

    const bitboard all_nopinned = potentially_pinned ^ all;

    const bitboard bsp_xray = bmoves_bb(tbl, ksqr, all_nopinned, 0);
    const bitboard rk_xray  = rmoves_bb(tbl, ksqr, all_nopinned, 0);

    const bitboard opps_bsp = opps[IDX_BISHP] | opps[IDX_QUEEN];
    const bitboard opps_rk  = opps[IDX_ROOK]  | opps[IDX_QUEEN];

    bitboard pinner = (bsp_xray & opps_bsp) | (rk_xray & opps_rk);

    bitboard blk = 0;

    for(; pinner; pinner = POP_LSB(pinner))
        blk |= tbl->between_bb[ksqr][TZCNT(pinner)];

    return blk;
}

void push_all(
        struct move_stack*      ms,
        const struct atts_tbl*  tbl,
        const bitboard          all,
        const bitboard*         us,
        const bitboard*         opps,
        const u32               ksqr,
        const bool              turn,
        const bitboard          target,
        const bitboard          mask,
        const bool              pinned,
        const u32               gentype)
{
    const bitboard all_opps = opps[IDX_ALL];
    const bitboard all_us   = us[IDX_ALL];

    const bitboard us_pawn  = us[IDX_PAWN];
    const bitboard us_night = us[IDX_NIGHT];
    const bitboard us_bishp = us[IDX_BISHP];
    const bitboard us_rook  = us[IDX_ROOK];
    const bitboard us_queen = us[IDX_QUEEN];

    push_sn(ms, tbl, all, all_us, all_opps, ksqr, target, us_queen & mask, pinned, gentype, qmoves_bb, NULL);
    push_sn(ms, tbl, all, all_us, all_opps, ksqr, target, us_rook  & mask, pinned, gentype, rmoves_bb, NULL);
    push_sn(ms, tbl, all, all_us, all_opps, ksqr, target, us_bishp & mask, pinned, gentype, bmoves_bb, NULL);
    push_sn(ms, tbl, all, all_us, all_opps, ksqr, target, us_night & mask, pinned, gentype, NULL, nmoves_bb);

    push_p(ms, tbl, all, all_opps, ksqr, target, us_pawn & mask, pinned, turn, gentype);
}

void genmoves_legal(
        const struct atts_tbl*  tbl,
        struct move_stack*      ms,
        const bool              turn,
        const bool              castle_k,
        const bool              castle_q,
        const u32               ep_target,
        const bitboard          all,
        const bitboard          bbs[2][7],
        const u32               gentype)
{
    const bitboard* us   = bbs[turn];
    const bitboard* opps = bbs[!turn];

    const u32 ksqr = TZCNT(us[IDX_KING]);

    const bitboard checkers = sqr_attrs_bb2(tbl, all, bbs, ksqr, !turn);
    const bitboard pinmask  = get_pinmask(tbl, all, us[IDX_ALL], opps, ksqr);

    push_k(ms, tbl, all, bbs, turn, ksqr, gentype);

    if(checkers) {
        if POP_LSB(checkers)
            return;

        const bitboard blk = checkers | tbl->between_bb[ksqr][TZCNT(checkers)];

        push_all(ms, tbl, all, us, opps, ksqr, turn, blk, ~pinmask, false, gentype);

        if(gentype != GENTYPE_QUIET)
            push_eep(ms, tbl, all, opps, us[IDX_PAWN], ksqr, ep_target, turn);

        return;
    }

    push_all(ms, tbl, all, us, opps, ksqr, turn, pinmask,  pinmask, true,  gentype);
    push_all(ms, tbl, all, us, opps, ksqr, turn, 0,       ~pinmask, false, gentype);

    if(gentype != GENTYPE_QUIET)
        push_eep(ms, tbl, all, opps, us[IDX_PAWN], ksqr, ep_target, turn);

    if(gentype != GENTYPE_CAPS) {
        if(castle_k)
            push_c(ms, tbl, all, bbs, ksqr, turn, false);

        if(castle_q)
            push_c(ms, tbl, all, bbs, ksqr, turn, true);
    }
}
