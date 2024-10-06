#include "position.h"


Position::Position() {
    turn = true;
    oo_w = ooo_w = oo_b = ooo_b = true;
    ep_target = false;

    bbs[w_pawn]   = rank_2;
    bbs[w_night]  = b1_bb | g1_bb; 
    bbs[w_bishop] = c1_bb | f1_bb; 
    bbs[w_rook]   = a1_bb | h1_bb; 
    bbs[w_queen]  = d1_bb; 
    bbs[w_king]   = e1_bb; 

    bbs[b_pawn]   = rank_7;
    bbs[b_night]  = b8_bb | g8_bb;
    bbs[b_bishop] = c8_bb | f8_bb;
    bbs[b_rook]   = a8_bb | h8_bb;
    bbs[b_queen]  = d8_bb;
    bbs[b_king]   = e8_bb;

    w_bb = b_bb = 0;
    for(int i = w_pawn; i <= b_king; ++i)
        (i <= w_king ? w_bb : b_bb) |= bbs[i];

    all_bb = w_bb | b_bb;
    us = &w_bb;
    them = &b_bb;
}
