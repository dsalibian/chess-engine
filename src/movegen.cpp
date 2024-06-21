#include "movegen.h"
#include "misc.h"

array<array<ull, 64>, 2> pawn_attacks;
array<ull, 64> night_attacks;
array<ull, 64> king_attacks;

array<array<ull, 4096>, 64> rook_attacks;
array<array<ull, 512>, 64> bishop_attacks;

void init_attacks() {
    for(int i = 0; i < 64; i++) {
        pawn_attacks[W][i] = pawn_attacks_mask(i, W);
        pawn_attacks[B][i] = pawn_attacks_mask(i, B);

        night_attacks[i] = night_attacks_mask(i);
        king_attacks[i] = king_attacks_mask(i);
    }
}

void init_magics();

std::mt19937_64 eng(chrono::high_resolution_clock::now().time_since_epoch().count());
std::uniform_int_distribution<unsigned long long> distr;
ull find_magic(int pos, bool bishop) {
    array<ull, 4096> moves, perms, done;
    
    ull relevant_mask = bishop ? bishop_relevant_mask(pos) : rook_relevant_mask(pos);
    int count = bit_count(relevant_mask);
    int u = 1 << count;

    for(int i = 0; i < u; i++) {
        perms[i] = relevant_occupancy_mask(i, count, relevant_mask);
        moves[i] = bishop ? bishop_attacks_mask(pos, perms[i]) : rook_attacks_mask(pos, perms[i]);  
    }

    for(;;) {
        retry:

        ull magic = ~( distr(eng) |  distr(eng) | distr(eng) );
        if( !magic ) continue;

        done.fill(0);
        
        for(int i = 0; i < u; i++) {
            int k = f(perms[i], magic, count);

            if( !done[k] )
                done[k] = moves[i];

            if( done[k] != moves[i] )
                goto retry;
        }
        return magic;
    }
}
ull relevant_occupancy_mask(int, int, ull);

ull relevant_occupancy_mask(int ind, int bit_count, ull attacks_mask) {
    ull occupancy = 0;
    
    for(int i = 0; i < bit_count; i++) {
        int p = pop_lsb(attacks_mask);

        if( ind & ( 1 << i ) )
            occupancy |= ( 1ULL << p );
    }

    return occupancy;
}

ull pawn_attacks_mask(int pos, bool side) {
    ull pos_board = 1ULL << pos;
    ull mask = 0;
    
    if( side == W ) {
        if( pos_board & ~a_file ) mask |= ( pos_board << 7 );
        if( pos_board & ~h_file ) mask |= ( pos_board << 9 );
        
    } else {
        if( pos_board & ~a_file ) mask |= ( pos_board >> 7 );
        if( pos_board & ~h_file ) mask |= ( pos_board >> 9 );
    }

    return mask;
}

ull night_attacks_mask(int pos) {
    ull pos_board = 1ULL << pos;
    ull mask = 0;
    
    if( pos_board & ~(ab_file | rank_8) ) mask |= ( pos_board << 6);
    if( pos_board & ~(a_file | rank_78) ) mask |= ( pos_board << 15);
    if( pos_board & ~(h_file | rank_78) ) mask |= ( pos_board << 17);
    if( pos_board & ~(gh_file | rank_8) ) mask |= ( pos_board << 10);
    if( pos_board & ~(gh_file | rank_1) ) mask |= ( pos_board >> 6);
    if( pos_board & ~(h_file | rank_12) ) mask |= ( pos_board >> 15);
    if( pos_board & ~(a_file | rank_12) ) mask |= ( pos_board >> 17);
    if( pos_board & ~(ab_file | rank_1) ) mask |= ( pos_board >> 10);

    return mask;
} 

ull bishop_relevant_mask(int pos) {
    ull mask = 0;

    int file = pos % 8;
    int rank = pos / 8;

    for(int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) mask |= ( 1ULL << ( 8 * r + f ) );
    for(int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) mask |= ( 1ULL << ( 8 * r + f ) );
    for(int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) mask |= ( 1ULL << ( 8 * r + f ) );
    for(int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) mask |= ( 1ULL << ( 8 * r + f ) );

    return mask;
} 

ull bishop_attacks_mask(int pos, ull block) {
    ull mask = 0;

    int file = pos % 8;
    int rank = pos / 8;

    for(int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) { 
        mask |= ( 1ULL << ( 8 * r + f ) ); 
        if( block & ( 1ULL << ( 8 * r + f ) ) )
            break;
    }
    for(int r = rank + 1, f = file - 1; r < 8 && f > -1; r++, f--) { 
        mask |= ( 1ULL << ( 8 * r + f ) ); 
        if( block & ( 1ULL << ( 8 * r + f ) ) )
            break;
    }
    for(int r = rank - 1, f = file + 1; r > -1 && f < 8; r--, f++) { 
        mask |= ( 1ULL << ( 8 * r + f ) ); 
        if( block & ( 1ULL << ( 8 * r + f ) ) )
            break;
    }
    for(int r = rank - 1, f = file - 1; r > -1 && f > -1; r--, f--) { 
        mask |= ( 1ULL << ( 8 * r + f ) ); 
        if( block & ( 1ULL << ( 8 * r + f ) ) )
            break;
    }

    return mask;
}

ull rook_relevant_mask(int pos) {
    ull mask = 0;

    int file = pos % 8;
    int rank = pos / 8;

    for(int r = rank + 1; r < 7; r++) mask |= ( 1ULL << ( 8 * r + file ) );
    for(int r = rank - 1; r > 0; r--) mask |= ( 1ULL << ( 8 * r + file ) );
    for(int f = file + 1; f < 7; f++) mask |= ( 1ULL << ( 8 * rank + f ) );
    for(int f = file - 1; f > 0; f--) mask |= ( 1ULL << ( 8 * rank + f ) );

    return mask;
} 

ull rook_attacks_mask(int pos, ull block) {
    ull mask = 0;

    int file = pos % 8;
    int rank = pos / 8;

    for(int r = rank + 1; r < 8; r++)  {
        mask |= ( 1ULL << ( 8 * r + file )); 
        if( block & ( 1ULL << ( 8 * r + file ) ) )
            break;    
    }
    for(int r = rank - 1; r > -1; r--)  {
        mask |= ( 1ULL << ( 8 * r + file )); 
        if( block & ( 1ULL << ( 8 * r + file ) ) )
            break;    
    }
    for(int f = file + 1; f < 8; f++)  {
        mask |= ( 1ULL << ( 8 * rank + f )); 
        if( block & ( 1ULL << ( 8 * rank + f ) ) )
            break;    
    }
    for(int f = file - 1; f > -1; f--)  {
        mask |= ( 1ULL << ( 8 * rank + f )); 
        if( block & ( 1ULL << ( 8 * rank + f ) ) )
            break;    
    }

    return mask;
}

ull queen_attacks_mask(int pos) {
    return bishop_relevant_mask(pos) | rook_relevant_mask(pos);
} 

ull king_attacks_mask(int pos) {
    ull pos_board = 1ULL << pos;
    ull mask = 0;

    if( pos_board & ~rank_8 ) mask |= ( pos_board << 8 );
    if( pos_board & ~rank_1 ) mask |= ( pos_board >> 8 );
    if( pos_board & ~a_file ) mask |= ( pos_board >> 1 );
    if( pos_board & ~h_file ) mask |= ( pos_board << 1 );

    if( pos_board & ~(rank_8 | h_file) ) mask |= ( pos_board << 9 );
    if( pos_board & ~(rank_8 | a_file) ) mask |= ( pos_board << 7 );
    if( pos_board & ~(rank_1 | a_file) ) mask |= ( pos_board >> 9 );
    if( pos_board & ~(rank_1 | h_file) ) mask |= ( pos_board >> 7 );

    return mask;
} 
