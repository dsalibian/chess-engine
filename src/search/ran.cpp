#include "ran.h"
#include <cstdint>
#include <chrono>

typedef std::uint64_t uint64;
typedef std::uint64_t bitboard;
typedef __uint128_t   uint128;

namespace ran {

void seed(uint64* state, int size) {
    splitmix m;
    for(int i = 0; i < size; ++i) 
        state[i] = m.next();
}



splitmix::splitmix() { 
    using namespace std::chrono;
    *state = unsigned(system_clock::now().time_since_epoch().count());
    for(int i = 0; i < 0x999; ++i) 
        *state = next(*state);
}

splitmix::splitmix(uint64 seed) { 
    *state = seed; 
}

uint64 splitmix::next() {
    uint64 t = (*state += 0x9e3779b97f4a7c15ULL);
    t = (t ^ (t >> 30)) * 0xbf58476d1ce4e5b9ULL;
    t = (t ^ (t >> 27)) * 0x94d049bb133111ebULL;
    return t ^ (t >> 31);
}

uint64 splitmix::next(uint64& state) {
    state += 0x9e3779b97f4a7c15ULL;
    state = (state ^ (state >> 30)) * 0xbf58476d1ce4e5b9ULL;
    state = (state ^ (state >> 27)) * 0x94d049bb133111ebULL;
    return state ^ (state >> 31);
}



xorshift::xorshift() { 
    seed(state, size); 
}

uint64 xorshift::next() {
    *state ^= *state << 7; 
    *state ^= *state >> 9; 
    return *state;
}



xorshiro::xorshiro() { 
    seed(state, size); 
};

uint64 xorshiro::rotl(uint64 x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64 xorshiro::next() {
    const uint64_t result = rotl(state[0] + state[3], 23) + state[0];
    const uint64_t t = state[1] << 17;
    state[2] ^= state[0];
    state[3] ^= state[1];
    state[1] ^= state[2];
    state[0] ^= state[3];
    state[2] ^= t;
    state[3] = rotl(state[3], 45);
    return result;
}



mwc::mwc() { 
    seed(state, size); 
    state[2] ? (state[2] &= MWC_A2 - 2) : state[2] = 1;
}

uint64 mwc::next() {
    const uint64 result = state[1];
    const uint128 t = MWC_A2 * state[0] + state[2];
    state[0] = state[1];
    state[1] = uint64(t);
    state[2] = uint64(t >> 64);
    return result;
}



pcg::pcg() { 
    seed(state, size); 
}

uint64 pcg::rotr64(uint64 x, unsigned k) {
    return (x >> k) | (x << ((-k) & 63));
}

uint64 pcg::rotr128(uint128 x) {
    return rotr64(uint64(x >> 64) ^ uint64(x), unsigned(x >> 122));
}

uint64 pcg::next() {
    uint128 s = (uint128(state[1]) << 64) | state[0];
    uint128 t = s * m + a;
    state[0] = uint64(t);
    state[1] = uint64(t >> 64);
    return rotr128(t);
}

} // namespace ran
