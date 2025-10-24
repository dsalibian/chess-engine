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

#define FILE_A  0x101010101010101ull
#define FILE_B  (FILE_A << 1)
#define FILE_C  (FILE_B << 1)
#define FILE_D  (FILE_C << 1)            
#define FILE_E  (FILE_D << 1)           
#define FILE_F  (FILE_E << 1)          
#define FILE_G  (FILE_F << 1)         
#define FILE_H  (FILE_G << 1)        
#define FILE_AH (FILE_A | FILE_H)       
                             
#define RANK_1  0xffull
#define RANK_2  (RANK_1 << 8)
#define RANK_3  (RANK_2 << 8)
#define RANK_4  (RANK_3 << 8)
#define RANK_5  (RANK_4 << 8)
#define RANK_6  (RANK_5 << 8)
#define RANK_7  (RANK_6 << 8)
#define RANK_8  (RANK_7 << 8)
#define RANK_18 (RANK_1 | RANK_8)

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

#endif
