#include "movegen.h"
#include "bits.h"
#include "position.h"

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





struct movegen_tbl gen_mg_tbl() {
    struct movegen_tbl tbl;

    u64 bsp_s = 0x6f67ea16a95f0393ull;
    u64 rk_s  = 0x9671405820c301a7ull;

    for(u32 s = 0; s < 64; ++s) {
        tbl.bsp_magic[s]  = magic_make(s, true,  &bsp_s);        
        tbl.rk_magic[s]   = magic_make(s, false, &rk_s);        

        tbl.patts[TURN_W][s] = gen_patts(s, TURN_W);
        tbl.patts[TURN_B][s] = gen_patts(s, TURN_B);

        tbl.natts[s] = gen_natts(s);
        tbl.katts[s] = gen_katts(s);
    }

    return tbl;
}
