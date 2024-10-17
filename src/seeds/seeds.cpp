#include <cmath>
#include <cstdint>
#include <iostream>
#include <atomic>
#include <pthread.h>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

typedef std::uint64_t uint64;
typedef std::uint64_t bitboard;
typedef __uint128_t   uint128;

namespace movegen {

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

enum Direction {
    north,
    east,
    south,
    west,
    neast,
    nwest,
    seast,
    swest,
};

constexpr int dir_delta[8][2] = {
    { 0,  1},
    { 1,  0},
    { 0, -1},
    {-1,  0},
    { 1,  1},
    {-1,  1},
    { 1, -1},
    {-1, -1},
};

bitboard ray(int pos, int d, bitboard block) {
    bitboard bb = 0;

    const int df = dir_delta[d][0];
    const int dr = dir_delta[d][1];

    unsigned f = (pos % 8) + df;
    unsigned r = (pos / 8) + dr;
    for( ; f <= 7 && r <= 7; f += df, r += dr) {
        bitboard t = 1ULL << (8 * r + f);
        bb |= t;
        if( t & block ) break;
    }
    
    return bb;
}

bitboard rmask(int pos, bool bsp) {
    bitboard mask = 0;
    constexpr bitboard r_bounds[4] {~rank_8, ~h_file, ~rank_1, ~a_file};
    constexpr bitboard b_bounds = ~(ah_file | rank_18);
    int offs = bsp ? 4: 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(pos, dir, 0) & (bsp ? b_bounds : r_bounds[dir]);

    return mask;
}

bitboard satts(int pos, uint64 block, bool bsp) {
    bitboard mask = 0;
    int offs = bsp ? 4 : 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(pos, dir, block);

    return mask;
}

} // namespace movegen


namespace bits {

bool get_bit(bitboard bb, int k) {
    return (bb >> k) & 1;
}

void pop_lsb(bitboard& bb) {
    bb &= bb - 1; 
}

bitboard occ_mask(unsigned index, int bitcount, bitboard atts) {
    bitboard occ = 0;

    for(int i = 0; i < bitcount; ++i) {
        int k = __builtin_ctzll(atts);
        pop_lsb(atts);

        if( index & (1U << i)) 
            occ |= (1ULL << k);
    }

    return occ;
}

} // namespace bits


enum Rk_Bsp {rook, bishop};
struct Tbls {
    bitboard**  rmask;
    int**       popcnt;
    int**       shamt;
    int**       u;
    bitboard*** perms;
    bitboard*** moves;

    Tbls(int){}
    Tbls() {
        rmask  = new bitboard*[64];
        popcnt = new int*[64];
        u      = new int*[64];
        shamt  = new int*[64];
        perms  = new bitboard**[64];
        moves  = new bitboard**[64];

        for(int s = 0; s < 64; ++s) {
            rmask[s]  = new bitboard[2];
            popcnt[s] = new int[2];
            u[s]      = new int[2];
            shamt[s]  = new int[2];
            perms[s]  = new bitboard*[2];
            moves[s]  = new bitboard*[2];

            for( int t: {rook, bishop} ) {
                rmask [s][t] = movegen::rmask(s, t);
                popcnt[s][t] = __builtin_popcountll(rmask[s][t]);
                u     [s][t] = 1U << popcnt[s][t];
                shamt [s][t] = 64  - popcnt[s][t];
                perms [s][t] = new bitboard[u[s][t]];
                moves [s][t] = new bitboard[u[s][t]];
            }

            for(int i = 0; i < u[s][rook]; ++i) {
                for( int t: {rook, bishop} ) {
                    if( i < u[s][t] ) {
                        perms[s][t][i] = bits::occ_mask(i, popcnt[s][t], rmask[s][t]);
                        moves[s][t][i] = movegen::satts(s, perms[s][t][i], t);
                    }
                }
            }
        }
    }
};

std::mutex m;

namespace ran {

// https://prng.di.unimi.it/splitmix64.c
struct splitmix {
    static constexpr int size = 1;
    uint64 state[size];

    splitmix(uint64 seed) { state[0] = seed; }

    uint64 next() {
        uint64 t = (state[0] += 0x9e3779b97f4a7c15ULL);
        t = (t ^ (t >> 30)) * 0xbf58476d1ce4e5b9ULL;
        t = (t ^ (t >> 27)) * 0x94d049bb133111ebULL;
        return t ^ (t >> 31);
    }
};

void seed(uint64* state, int size) {
    splitmix m(random());
    for(int i = 0; i < 999; ++i) 
        m.next();

    for(int i = 0; i < size; ++i) 
        state[i] = m.next();
}

// https://en.wikipedia.org/wiki/Xorshift#Example_implementation
struct xorshift {
    static constexpr int size = 1;
    uint64 state[size];

    xorshift() { seed(state, size); }

    uint64 next() {
        state[0] ^= state[0] << 7; 
        state[0] ^= state[0] >> 9; 
        return state[0]; 
    }
};

// https://prng.di.unimi.it/xoshiro256plusplus.c
struct xorshiro {
    static constexpr int size = 4;
    uint64 state[size];    

    xorshiro() { seed(state, size); }

    static uint64 rotl(uint64 x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    uint64 next() {
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
};

// https://prng.di.unimi.it/MWC192.c
struct mwc {
    static constexpr int size = 3;
    uint64 state[size];

    mwc() { 
        seed(state, size - 1); 
        state[2] = 1; 
    }

    static constexpr uint128 MWC_A2 = 0xffa04e67b3c95d86ULL;

    uint64 next() {
        const uint64 result = state[1];
        const uint128 t = MWC_A2 * state[0] + state[2];
        state[0] = state[1];
        state[1] = uint64(t);
        state[2] = uint64(t >> 64);
        return result;
    }
};

// https://github.com/imneme/pcg-cpp/blob/master/include/pcg_random.hpp
// https://github.com/lemire/testingRNG/blob/master/source/pcg64.h#L20
struct pcg {
    static constexpr int size = 2; 
    uint64 state[2];
    
    pcg() { seed(state, 2); };

    static constexpr uint128 m = (uint128(2549297995355413924ULL) << 64) | 4865540595714422341ULL;
    static constexpr uint128 a = (uint128(6364136223846793005ULL) << 64) | 1442695040888963407ULL;

    uint64 rotr64(uint64 x, unsigned k) {
        return (x >> k) | (x << ((-k) & 63));
    }

    uint64 rotr128(uint128 x) {
        return rotr64(uint64(x >> 64) ^ uint64(x), unsigned(x >> 122));
    }
    
    uint64 next() {
        uint128 s = (uint128(state[1]) << 64) | state[0];
        uint128 t = s * m + a;
        state[0] = uint64(t);
        state[1] = uint64(t >> 64);
        return rotr128(t);
    }
};

} // namespace ran

std::atomic<unsigned> global_min{UINT32_MAX};

template<typename Ran>
struct Searcher {
    Tbls* tbl;
    bitboard* atts;
    unsigned* iters, total_its = 0;
    uint64* pstate;
    Ran ran{};

    Searcher(Tbls* _tbl) {
        tbl    = _tbl;
        atts   = new bitboard[1U << 12];
        iters  = new unsigned[1U << 12];
        pstate = new uint64[ran.size];
    }

    bool valid_magic(int pos, bool bsp, uint64 magic) {
        ++total_its;
        for(int j = 0; j < tbl->u[pos][bsp]; ++j) {
            unsigned k = unsigned((magic * tbl->perms[pos][bsp][j]) >> tbl->shamt[pos][bsp]);

            if( iters[k] != total_its ) {
                iters[k] = total_its;
                atts [k] = tbl->moves[pos][bsp][j];
            }
            else if( atts[k] != tbl->moves[pos][bsp][j] ) 
                return false;
        }
        return true;
    }

    uint64 sparse() {
        return ran.next() & ran.next() & ran.next();
    }

    void find_magic(int pos, bool bishop, unsigned& cur_it, unsigned min) {
        uint64 magic;
        for(;; ++cur_it) {
            do magic = sparse();
            while (!magic);

            if( cur_it > min || valid_magic(pos, bishop, magic )) 
                return;
        }
    }

    int find_all(unsigned min) {
        unsigned cur_it = 0;

        for(int i = 0; i < 64; ++i) {
            for(int k: {0, 1}) {
                find_magic(i, k, cur_it, min);
                if( cur_it > min ) 
                    return cur_it;
            }
        }

        return cur_it;
    }

    void write_pstate() {
        for(int i = 0; i < ran.size; ++i)
            pstate[i] = ran.state[i];
    }

    void read_pstate() {
        for(int i = 0; i < ran.size; ++i)
            ran.state[i] = pstate[i];
    }

    void jmp() {
        do write_pstate();
        while ( !valid_magic(0, 0, sparse()) );
        read_pstate();
    }

    void search() {
        using namespace std;
        for(;;) {
            jmp();
            unsigned c = find_all(global_min);
            if( c < global_min ) {
                cout << "seeds: ";
                for(int i = 0; i < ran.size; ++i)
                    cout << "s[" << dec << i << "] = 0x" << hex << pstate[i] << "   ";
                cout << "\niters = " << dec << c << endl << endl;
                global_min = c;
            } 
        }
    }
};


template<typename Ran>
void start_search(int thread_count) {
    using std::vector, std::thread;

    Tbls tbl{};
    vector<thread> v;

    for(int i = 0; i < thread_count; ++i)
        v.push_back(thread(&Searcher<Ran>::search, Searcher<Ran>(&tbl)));

    for(int i = 0; i < thread_count; ++i)
        v[i].join();
}

int main() {
    using namespace std;
    srandom(unsigned(std::chrono::system_clock::now().time_since_epoch().count()));

    start_search<ran::xorshift>(1);

    return 0;
}
