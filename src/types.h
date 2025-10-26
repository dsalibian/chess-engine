#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

typedef u64 bitboard;

#define FILE_A  0x0101010101010101ull
#define FILE_B  0x0202020202020202ull
#define FILE_C  0x0404040404040404ull
#define FILE_D  0x0808080808080808ull
#define FILE_E  0x1010101010101010ull
#define FILE_F  0x2020202020202020ull
#define FILE_G  0x4040404040404040ull
#define FILE_H  0x8080808080808080ull
#define FILE_AH 0x8181818181818181ull

#define RANK_1  0x00000000000000ffull
#define RANK_2  0x000000000000ff00ull
#define RANK_3  0x0000000000ff0000ull
#define RANK_4  0x00000000ff000000ull
#define RANK_5  0x000000ff00000000ull
#define RANK_6  0x0000ff0000000000ull
#define RANK_7  0x00ff000000000000ull
#define RANK_8  0xff00000000000000ull
#define RANK_18 0xff000000000000ffull

enum square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

enum occupancy {
    OCCUPANCY_W_PAWN,
    OCCUPANCY_W_NIGHT,
    OCCUPANCY_W_BISHP,
    OCCUPANCY_W_ROOK,
    OCCUPANCY_W_QUEEN,
    OCCUPANCY_W_KING,
   
    OCCUPANCY_B_PAWN,
    OCCUPANCY_B_NIGHT,
    OCCUPANCY_B_BISHP,
    OCCUPANCY_B_ROOK,
    OCCUPANCY_B_QUEEN,
    OCCUPANCY_B_KING,

    OCCUPANCY_EMPTY,
};

enum piece_type {
    TYPE_PAWN, 
    TYPE_NIGHT,
    TYPE_BISHP,
    TYPE_ROOK,
    TYPE_QUEEN,
    TYPE_KING,

    TYPE_NONE,
};

#endif
