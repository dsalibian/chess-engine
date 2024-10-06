#ifndef TYPES_H
#define TYPES_H

#include <cassert>
#include <cstdint>
#include <array>

typedef uint64_t uint64;
typedef uint64_t bitboard;

enum Occupancy {
    w_pawn,
    w_night,
    w_bishop,
    w_rook,
    w_queen,
    w_king,

    b_pawn,
    b_night,
    b_bishop,
    b_rook,
    b_queen,
    b_king,

    empty,
};

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1, 
    a2, b2, c2, d2, e2, f2, g2, h2, 
    a3, b3, c3, d3, e3, f3, g3, h3, 
    a4, b4, c4, d4, e4, f4, g4, h4, 
    a5, b5, c5, d5, e5, f5, g5, h5, 
    a6, b6, c6, d6, e6, f6, g6, h6, 
    a7, b7, c7, d7, e7, f7, g7, h7, 
    a8, b8, c8, d8, e8, f8, g8, h8, 
};

enum Square_BB {
    a1_bb = 1ULL<< a1, b1_bb = 1ULL << b1, c1_bb = 1ULL << c1, d1_bb = 1ULL << d1, e1_bb = 1ULL << e1, f1_bb = 1ULL << f1, g1_bb = 1ULL << g1, h1_bb = 1ULL << h1,
    a2_bb = 1ULL<< a2, b2_bb = 1ULL << b2, c2_bb = 1ULL << c2, d2_bb = 1ULL << d2, e2_bb = 1ULL << e2, f2_bb = 1ULL << f2, g2_bb = 1ULL << g2, h2_bb = 1ULL << h2,
    a3_bb = 1ULL<< a3, b3_bb = 1ULL << b3, c3_bb = 1ULL << c3, d3_bb = 1ULL << d3, e3_bb = 1ULL << e3, f3_bb = 1ULL << f3, g3_bb = 1ULL << g3, h3_bb = 1ULL << h3,
    a4_bb = 1ULL<< a4, b4_bb = 1ULL << b4, c4_bb = 1ULL << c4, d4_bb = 1ULL << d4, e4_bb = 1ULL << e4, f4_bb = 1ULL << f4, g4_bb = 1ULL << g4, h4_bb = 1ULL << h4,
    a5_bb = 1ULL<< a5, b5_bb = 1ULL << b5, c5_bb = 1ULL << c5, d5_bb = 1ULL << d5, e5_bb = 1ULL << e5, f5_bb = 1ULL << f5, g5_bb = 1ULL << g5, h5_bb = 1ULL << h5,
    a6_bb = 1ULL<< a6, b6_bb = 1ULL << b6, c6_bb = 1ULL << c6, d6_bb = 1ULL << d6, e6_bb = 1ULL << e6, f6_bb = 1ULL << f6, g6_bb = 1ULL << g6, h6_bb = 1ULL << h6,
    a7_bb = 1ULL<< a7, b7_bb = 1ULL << b7, c7_bb = 1ULL << c7, d7_bb = 1ULL << d7, e7_bb = 1ULL << e7, f7_bb = 1ULL << f7, g7_bb = 1ULL << g7, h7_bb = 1ULL << h7,
    a8_bb = 1ULL<< a8, b8_bb = 1ULL << b8, c8_bb = 1ULL << c8, d8_bb = 1ULL << d8, e8_bb = 1ULL << e8, f8_bb = 1ULL << f8, g8_bb = 1ULL << g8, h8_bb = 1ULL << h8,
};

enum Files_Ranks {
    a_file  = 0x0101010101010101ULL,
    b_file  = 0x0202020202020202ULL, 
    g_file  = 0x4040404040404040ULL, 
    h_file  = 0x8080808080808080ULL, 
    ab_file = 0x0303030303030303ULL, 
    gh_file = 0xc0c0c0c0c0c0c0c0ULL, 
    ah_file = 0x8181818181818181ULL, 

    rank_1  = 0x00000000000000ffULL,
    rank_2  = 0x000000000000ff00ULL,
    rank_7  = 0x00ff000000000000ULL,
    rank_8  = 0xff00000000000000ULL,
    rank_12 = 0x000000000000ffffULL,
    rank_78 = 0xffff000000000000ULL,
    rank_18 = 0xff000000000000ffULL,
};

// struct xorshift {
//     uint64 state;
//     uint64 next() {
//         state ^= state << 7; 
//         state ^= state >> 9; 
//         return state; 
//     }
//     uint64 sparse() { 
//         return state & next() & next(); 
//     }
// };


struct xorshiro {
    uint64 state[2];
    xorshiro(uint64 s0, uint64 s1, int n) {
        state[0] = s0;
        state[1] = s1;
        for(int i = 0; i < n; ++i) jump();
    }
    xorshiro(){}

    uint64 rotl(uint64 x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    
    uint64 next() {
        const uint64 s0 = state[0];
        uint64 s1 = state[1];
        const uint64 result = rotl(s0 + s1, 17) + s0;
        
        s1 ^= s0;
        state[0] = rotl(s0, 49) ^ (s1 << 21);
        state[1] = rotl(s1, 28);

        return result;
    }

    uint64 sparse() {
        return state[0] & state[1] & next();
    }

    void jump() {
        constexpr uint64 JUMP[] = { 0x2bd7a6a6e99c2ddcULL, 0x0992ccaf6a6fca05ULL };

        uint64 s0 = 0, s1 = 0;
        for(unsigned i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
            for(unsigned b = 0; b < 64; b++) {
                if (JUMP[i] & 1ULL << b) {
                    s0 ^= state[0];
                    s1 ^= state[1];
                }
                next();
            }

        state[0] = s0;
        state[1] = s1;
    }
};

#endif
