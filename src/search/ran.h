#ifndef RAN_H
#define RAN_H

#include <cstdint> 

typedef std::uint64_t uint64;
typedef std::uint64_t bitboard;
typedef __uint128_t   uint128;

namespace ran {

// https://prng.di.unimi.it/splitmix64.c
struct splitmix {
    static constexpr int size = 1;
    uint64 state[size];

    splitmix(uint64);
    uint64 next();
    static uint64 next(uint64&);
};

// https://en.wikipedia.org/wiki/Xorshift#Example_implementation
struct xorshift {
    static constexpr int size = 1;
    uint64 state[size];

    xorshift();
    uint64 next();
};

// https://prng.di.unimi.it/xoshiro256plusplus.c
struct xorshiro {
    static constexpr int size = 4;
    uint64 state[size];    

    xorshiro();
    uint64 rotl(uint64, int);
    uint64 next();
};

// https://prng.di.unimi.it/MWC192.c
struct mwc {
    static constexpr int size = 3;
    static constexpr uint128 MWC_A2 = 0xffa04e67b3c95d86ULL;
    uint64 state[size];

    mwc();
    uint64 next();
};

// https://github.com/imneme/pcg-cpp/blob/master/include/pcg_random.hpp
// https://github.com/lemire/testingRNG/blob/master/source/pcg64.h#L20
struct pcg {
    static constexpr int size = 2; 
    static constexpr uint128 m = (uint128(2549297995355413924ULL) << 64) | 4865540595714422341ULL;
    static constexpr uint128 a = (uint128(6364136223846793005ULL) << 64) | 1442695040888963407ULL;
    uint64 state[2];
    
    pcg();
    uint64 rotr64(uint64, unsigned);
    uint64 rotr128(uint128); 
    uint64 next();
};

void seed(uint64*, int);

} // namespace ran

#endif
