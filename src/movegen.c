#include "movegen.h"
#include "bits.h"

#define NIGHTJMP_NW_W(s) ( SQR_FILE(s) > FILE_B ? SQR_BB(s) <<  6 : 0 )
#define NIGHTJMP_NW_N(s) ( SQR_FILE(s) > FILE_A ? SQR_BB(s) << 15 : 0 )
#define NIGHTJMP_NE_N(s) ( SQR_FILE(s) < FILE_H ? SQR_BB(s) << 17 : 0 )
#define NIGHTJMP_NE_E(s) ( SQR_FILE(s) < FILE_G ? SQR_BB(s) << 10 : 0 )

#define NIGHTJMP_SE_E(s) ( SQR_FILE(s) < FILE_G ? SQR_BB(s) >>  6 : 0 )
#define NIGHTJMP_SE_S(s) ( SQR_FILE(s) < FILE_H ? SQR_BB(s) >> 15 : 0 )
#define NIGHTJMP_SW_S(s) ( SQR_FILE(s) > FILE_A ? SQR_BB(s) >> 17 : 0 )
#define NIGHTJMP_SW_W(s) ( SQR_FILE(s) > FILE_B ? SQR_BB(s) >> 10 : 0 )

#define KNGMV_N(s) ( SQR_BB(s) << 8)
#define KNGMV_S(s) ( SQR_BB(s) >> 8)
#define KNGMV_E(s) ( SQR_FILE(s) < FILE_H ? SQR_BB(s) << 1 : 0 )
#define KNGMV_W(s) ( SQR_FILE(s) > FILE_A ? SQR_BB(s) >> 1 : 0 )

#define KNGMV_NE(s) ( SQR_FILE(s) < FILE_H ? SQR_BB(s) << 9 : 0 )
#define KNGMV_NW(s) ( SQR_FILE(s) > FILE_A ? SQR_BB(s) << 7 : 0 )
#define KNGMV_SE(s) ( SQR_FILE(s) < FILE_H ? SQR_BB(s) >> 7 : 0 )
#define KNGMV_SW(s) ( SQR_FILE(s) > FILE_A ? SQR_BB(s) >> 9 : 0 )

#define PWNATTS_W(s) ( KNGMV_NE(s) | KNGMV_NW(s) )
#define PWNATTS_B(s) ( KNGMV_SE(s) | KNGMV_SW(s) )

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

bitboard _gen_satts(const u32 sqr, const bool bsp, const bitboard blk) {
    assert(sqr < 64);

    const i32 df[8] = { 0,  1,  0, -1,  1, -1,  1, -1};
    const i32 dr[8] = { 1,  0, -1,  0,  1,  1, -1, -1};

    bitboard atts = 0;

    for(i32 dir = bsp ? NORTH_EAST : NORTH; dir <= (bsp ? SOUTH_WEST : WEST); ++dir)
        for(i32 f = SQR_FILE(sqr), r = SQR_RANK(sqr); (u32)(f += df[dir]) < 8 && (u32)(r += dr[dir]) < 8; )
            if((atts |= SQR_BB(8 * r + f)) & blk)
                break;
    
    return atts; 
}

bitboard gen_ratts(const u32 sqr, const bitboard blk) {
    return _gen_satts(sqr, false, blk);
}

bitboard gen_batts(const u32 sqr, const bitboard blk) {
    return _gen_satts(sqr, true, blk);
}

bitboard gen_patts(const u32 sqr, const bool w) {
    return w ? PWNATTS_W(sqr) : PWNATTS_B(sqr);
}

bitboard gen_natts(const u32 sqr) {
    return 
        NIGHTJMP_NW_N(sqr) | NIGHTJMP_SW_S(sqr) |
        NIGHTJMP_NW_W(sqr) | NIGHTJMP_SW_W(sqr) |
        NIGHTJMP_NE_E(sqr) | NIGHTJMP_SE_E(sqr) |
        NIGHTJMP_NE_N(sqr) | NIGHTJMP_SE_S(sqr);
}

bitboard gen_katts(const u32 sqr) {
    return 
        KNGMV_NW(sqr) | KNGMV_N(sqr) | KNGMV_NE(sqr) |
        KNGMV_W (sqr) |                KNGMV_E (sqr) |
        KNGMV_SW(sqr) | KNGMV_S(sqr) | KNGMV_SE(sqr);
}
