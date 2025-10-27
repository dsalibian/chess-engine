#include "movegen.h"
#include "bits.h"
#include "move.h"
#include "position.h"
#include "misc.h"

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

bitboard gen_satts(const u32 sqr, const bool bsp, const bitboard blk) {
    const i32 df[8] = { 0,  1,  0, -1,  1, -1,  1, -1};
    const i32 dr[8] = { 1,  0, -1,  0,  1,  1, -1, -1};

    bitboard atts = 0;

    for(i32 dir = bsp ? NORTH_EAST : NORTH; dir <= (bsp ? SOUTH_WEST : WEST); ++dir) {
        for(i32 f = SQR_FILE(sqr), r = SQR_RANK(sqr); (u32)(f += df[dir]) < 8 && (u32)(r += dr[dir]) < 8; ) {
            bitboard s = SQR_BB(8 * r + f);
            atts |= s;

            if(s & blk)
                break;
        }
    }
    
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

        tbl.patts[TURN_W][s] = gen_patts(s, TURN_W);
        tbl.patts[TURN_B][s] = gen_patts(s, TURN_B);

        tbl.natts[s] = gen_natts(s);
        tbl.katts[s] = gen_katts(s);

        tbl.ppush[TURN_W][s] = SQR_BB(s) << 8;
        tbl.ppush[TURN_B][s] = SQR_BB(s) >> 8;

        if(SQR_RANK(s) == 1)
            tbl.dpush_check[TURN_W][s] = tbl.ppush[TURN_W][s] | (tbl.dpush[TURN_W][s] = SQR_BB(s) << 16);

        if(SQR_RANK(s) == 6)
            tbl.dpush_check[TURN_B][s] = tbl.ppush[TURN_B][s] | (tbl.dpush[TURN_B][s] = SQR_BB(s) >> 16);

    }

    // king castle bitboards
    {
        tbl.castle_bb[TURN_W][0] = SQR_BB(E1) | SQR_BB(G1); // kingside
        tbl.castle_bb[TURN_W][1] = SQR_BB(E1) | SQR_BB(C1); // queenside

        tbl.castle_bb[TURN_B][0] = SQR_BB(E8) | SQR_BB(G8);
        tbl.castle_bb[TURN_B][1] = SQR_BB(E8) | SQR_BB(C8);
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

    // castling 
    {
        // kingside squares
        tbl.castle_path[TURN_W][0][0] = E1;
        tbl.castle_path[TURN_W][0][1] = F1;
        tbl.castle_path[TURN_W][0][2] = G1;

        tbl.castle_path[TURN_B][0][0] = E8;
        tbl.castle_path[TURN_B][0][1] = F8;
        tbl.castle_path[TURN_B][0][2] = G8;

        // queenside squares
        tbl.castle_path[TURN_W][1][0] = E1;
        tbl.castle_path[TURN_W][1][1] = D1;
        tbl.castle_path[TURN_W][1][2] = C1;
                   
        tbl.castle_path[TURN_B][1][0] = E8;
        tbl.castle_path[TURN_B][1][1] = D8;
        tbl.castle_path[TURN_B][1][2] = C8;

        // kingside path bitboards
        tbl.castle_path_bb[TURN_W][0] = SQR_BB(F1) | SQR_BB(G1);
        tbl.castle_path_bb[TURN_B][0] = SQR_BB(F8) | SQR_BB(G8);

        // queenside path bitboards
        tbl.castle_path_bb[TURN_W][1] = SQR_BB(B1) | SQR_BB(C1) | SQR_BB(D1);
        tbl.castle_path_bb[TURN_B][1] = SQR_BB(B8) | SQR_BB(C8) | SQR_BB(D8);
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
    return tbl->patts[turn][sqr] & SQR_BB(target);
}


static void push_slider(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const bitboard all,
    const bitboard opps_all,
    const bitboard* us,
    const bitboard piece_idx,
    bitboard (*f)(const struct atts_tbl*, const u32, const bitboard, const bitboard))
{
    bitboard bb = us[piece_idx];                                                      

    for(; bb; bb = POP_LSB(bb)){                                                            
        u32 from = TZCNT(bb);                                                               
        bitboard moves = f(tbl, from, all, us[IDX_ALL]);                       

        for(; moves; moves = POP_LSB(moves)) {                                              
            u32 to = TZCNT(moves);                                                          
            u32 code = ((opps_all) & SQR_BB(to)) ? MCODE_CAPTURE : MCODE_QUIET;              

            stack_push(ms, (u16)MV_ENCODE(code, to, from));                          
        }                                                                                   
    }                                                                                       
}

static void push_kn(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const bitboard opps_all,
    const bitboard* us,
    const bitboard piece_idx,
    bitboard (*f)(const struct atts_tbl*, const u32, const bitboard))
{
    bitboard bb = us[piece_idx];                                        

    for(; bb; bb = POP_LSB(bb)){                                              
        u32 from = TZCNT(bb);                                                 
        bitboard moves = f(tbl, from, us[IDX_ALL]);                 
                                                                              
        for(; moves; moves = POP_LSB(moves)) {                                
            u32 to = TZCNT(moves);                                            
            u32 code = ((opps_all) & SQR_BB(to)) ? MCODE_CAPTURE : MCODE_QUIET;
                                                                              
            stack_push(ms, (u16)MV_ENCODE(code, to, from));            
        }                                                                     
    }                                                                         
}

static void push_ppush(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const bitboard all,
    const bitboard* us,
    const bool turn,
    const bool promo)
{
    bitboard bb = us[IDX_PAWN];                                                   
                                                                                        
    if(promo) bb &= (turn) ? RANK_7 : RANK_2;                                           
    else      bb &= (turn) ? ~RANK_7 : ~RANK_2;                                         
                                                                                        
    const u32 code = (promo) ? MCODE_FPROMO : MCODE_QUIET;                                    
                                                                                        
    for(; bb; bb = POP_LSB(bb)) {                                                       
        u32 from = TZCNT(bb);                                                           
        bitboard moves = pmoves_push_bb(tbl, from, all, turn);                   
                                                                                        
        for(; moves; moves = POP_LSB(moves)) {                                          
            if(promo)                                                                   
                for(u32 i = 0; i < 4; ++i)                                              
                    stack_push(ms, (u16)MV_ENCODE(code + i, TZCNT(moves), from));
            else                                                                        
                stack_push(ms, (u16)MV_ENCODE(code, TZCNT(moves), from));        
        }                                                                               
    }                                                                                   
}

static void push_pcap(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const bitboard opps_all,
    const bitboard* us,
    const bool turn,
    const bool promo)
{
    bitboard bb = us[IDX_PAWN];                                                   
                                                                                        
    if(promo) bb &= (turn) ? RANK_7 : RANK_2;                                           
    else      bb &= (turn) ? ~RANK_7 : ~RANK_2;                                         
                                                                                        
    const u32 code = MCODE_FCAP | (promo ? MCODE_FPROMO : 0);
                                                                                        
    for(; bb; bb = POP_LSB(bb)) {                                                       
        u32 from = TZCNT(bb);                                                           
        bitboard moves = pmoves_cap_bb(tbl, from, opps_all, turn);                   
                                                                                        
        for(; moves; moves = POP_LSB(moves)) {                                          
            if(promo)                                                                   
                for(u32 i = 0; i < 4; ++i)                                              
                    stack_push(ms, (u16)MV_ENCODE(code + i, TZCNT(moves), from));
            else                                                                        
                stack_push(ms, (u16)MV_ENCODE(code, TZCNT(moves), from));        
        }                                                                               
    }                                                                                   
}

static void push_dpush(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const bitboard all,
    const bitboard* us,
    const bool turn)
{
    bitboard bb = us[IDX_PAWN] & (turn ? RANK_2 : RANK_7);

    const u32 code = MCODE_DBL_PUSH;

    for(; bb; bb = POP_LSB(bb)) {                                                       
        u32 from = TZCNT(bb);                                                           
        bitboard moves = pmoves_dpush_bb(tbl, from, all, turn);                   
                                                                                        
        for(; moves; moves = POP_LSB(moves)) 
            stack_push(ms, (u16)MV_ENCODE(code, TZCNT(moves), from));        
    }                                                                                   
}

static void push_ep(
    struct move_stack* ms,
    const struct atts_tbl* tbl,
    const u32 target,
    const bitboard* us,
    const bool turn)
{
    bitboard bb = us[IDX_PAWN] & (turn ? RANK_5 : RANK_4);

    const u32 code = MCODE_EN_PASSANT;

    for(; bb; bb = POP_LSB(bb)) {                                                       
        u32 from = TZCNT(bb);                                                           
        bitboard moves = pmoves_ep_bb(tbl, from, target, turn);                   
                                                                                        
        for(; moves; moves = POP_LSB(moves)) 
            stack_push(ms, (u16)MV_ENCODE(code, TZCNT(moves), from));        
    }                                                                                   
}

static void push_castling(
        struct move_stack* ms,
        const struct atts_tbl* tbl,
        const bitboard all,
        const bitboard* opps,
        const bool turn,
        const bool k)
{
    const u32 from = tbl->castle_path[turn][!k][0];
    const u32 trvl = tbl->castle_path[turn][!k][1];
    const u32 to   = tbl->castle_path[turn][!k][2];

    const bool path_clear = !(all & tbl->castle_path_bb[turn][!k]);

    const bool check = 
        sqr_attd_opps(tbl, from, all, opps, turn) || 
        sqr_attd_opps(tbl, trvl, all, opps, turn) || 
        sqr_attd_opps(tbl, to,   all, opps, turn);

    if(path_clear && !check)
        stack_push(ms, (u16)MV_ENCODE(k ? MCODE_CASTLE_K : MCODE_CASTLE_Q, to, from));
    
}

void stack_push_moves(struct move_stack* ms, const struct atts_tbl* tbl, const struct position* pos) {
    const bool turn         = pos->turn;

    const bitboard all      = pos->bb_all;
    const bitboard* us      = pos->bbs[turn];;
    const bitboard* opps    = pos->bbs[!turn];;

    // sliders
    push_slider(ms, tbl, all, opps[IDX_ALL], us, IDX_BISHP, bmoves_bb);
    push_slider(ms, tbl, all, opps[IDX_ALL], us, IDX_ROOK,  rmoves_bb);
    push_slider(ms, tbl, all, opps[IDX_ALL], us, IDX_QUEEN, qmoves_bb);

    // king and night
    push_kn(ms, tbl, opps[IDX_ALL], us, IDX_NIGHT, nmoves_bb);
    push_kn(ms, tbl, opps[IDX_ALL], us, IDX_KING,  kmoves_bb);

    // regular push/cap
    push_ppush(ms, tbl, all, us, turn, false);
    push_pcap(ms, tbl, opps[IDX_ALL], us, turn, false);

    // dbl push
    push_dpush(ms, tbl, all, us, turn);

    // en-passant
    if(pos->en_passant_target)
        push_ep(ms, tbl, pos->en_passant_target, us, turn);

    // promo push/cap
    push_ppush(ms, tbl, all, us, turn, true);
    push_pcap(ms, tbl, opps[IDX_ALL], us, turn, true);

    // castling
    if(pos->castle_k[turn])
        push_castling(ms, tbl, all, opps, turn, true);

    if(pos->castle_q[turn])
        push_castling(ms, tbl, all, opps, turn, false);
}
