#include "position.h"

Position::Position() {
    bitboards.fill(0);
    all_pieces = w_pieces = b_pieces = 0;
    
    bitboards[w_pawn] = rank_1 << 8;
    bitboards[b_pawn] = rank_1 << (8*6);

    bitboards[w_night] = BB(b1) | BB(g1);
    bitboards[b_night] = bitboards[w_night] << (8*7);
   
    bitboards[w_bishop] = BB(c1) | BB(f1);
    bitboards[b_bishop] = bitboards[w_bishop] << (8*7);
    
    bitboards[w_rook] = BB(a1) | BB(h1);
    bitboards[b_rook] = bitboards[w_rook] << (8*7);

    bitboards[w_queen] = BB(d1);
    bitboards[b_queen] = BB(d8);

    bitboards[w_king] = BB(e1);
    bitboards[b_king] = BB(e8);

    for(int i = 0; i < 12; i++) {
        if( i < 6 ) 
            w_pieces |= bitboards[i];
        else 
            b_pieces |= bitboards[i];

        all_pieces |= bitboards[i];
    }
}
