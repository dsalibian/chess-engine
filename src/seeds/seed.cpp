#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

typedef std::uint64_t uint64;
typedef std::uint64_t bitboard;

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

bitboard patts(int pos, bool turn) {
    bitboard mask = 0;
    constexpr int delta[2] {1, -1};
    unsigned f = pos % 8;
    unsigned r = pos / 8 + (turn ? 1 : -1);

    for(int df: delta)
        if( f + df <= 7 && r <= 7 ) 
            mask |= 1ULL << (8 * r + f + df);

    return mask;
}

bitboard natts(int pos) {
    bitboard mask = 0;
    constexpr int delta[4] {-2, -1, 1, 2};
    unsigned f = pos % 8;
    unsigned r = pos / 8;

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df, rank = r + dr;
            if( abs(df) == abs(dr) || file > 7 || rank > 7 ) continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

    return mask;
}

bitboard katts(int pos) {
    bitboard mask = 0;
    constexpr int delta[4] {-1, 0, 1};
    unsigned f = pos % 8;
    unsigned r = pos / 8;

    for(int df: delta) {
        for(int dr: delta) {
            unsigned file = f + df, rank = r + dr;
            if( (!df && !dr) || file > 7 || rank > 7 ) continue;
            
            mask |= 1ULL << (8 * rank + file); 
        }
    }

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

void print_bb(bitboard bb) {
    using std::cout;

    for(int r = 7; r >= 0; --r) {
        cout << ' ' << r + 1 << "   ";
        for(int f = 0; f <= 7; ++f) 
            cout << (bits::get_bit(bb, 8 * r + f) ? "1 " : ". ");
        cout << '\n';
    }
    cout << "\n     a b c d e f g h\n\n\n";
}


namespace ran {

namespace splitmix {

uint64 next(uint64* state) {
    uint64 t = ((*state) += 0x9e3779b97f4a7c15ULL);
    t = (t ^ (t >> 30)) * 0xbf58476d1ce4e5b9ULL;
    t = (t ^ (t >> 27)) * 0x94d049bb133111ebULL;
    return t ^ (t >> 31);
}

} // namespace splitmix

namespace xorshift {

uint64 next(uint64* state) {
    (*state) ^= (*state) << 7; 
    (*state) ^= (*state) >> 9; 
    return (*state); 
}

} //namespace xorshift

namespace xorshiro {

uint64 rotl(uint64 x, int k) {
    return (x << k) | (x >> (64 - k));
}

uint64 next(uint64* state) {
    const uint64 s0 = state[0];
    uint64 s1 = state[1];
    const uint64 result = rotl(s0 + s1, 17) + s0;

    s1 ^= s0;
    state[0] = rotl(s0, 49) ^ (s1 << 21);
    state[1] = rotl(s1, 28);

    return result;
}

} //namespace xorshiro

namespace mwc {
// seed with: 
// 0 < state[2] < MWC_A2 - 1

constexpr __uint128_t MWC_A2 = 0xffa04e67b3c95d86ULL;

uint64 next(uint64* state) {
    const uint64 result = state[1];
    const __uint128_t t = MWC_A2 * state[0] + state[2];
    state[0] = state[1];
    state[1] = uint64(t);
    state[2] = uint64(t >> 64);
    return result;
}

} // namespace mwc

template <typename func> 
uint64 sparse(func next, uint64* state) {
    return next(state) & next(state) & next(state);
}

} //namespace ran


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

std::atomic<int> global_min{ INT32_MAX };

template<typename ran_fun>
struct Searcher {
    Tbls* tbl;
    bitboard* atts;
    uint64* iters, total_its = 0;
    ran_fun next;

    uint64* state;
    uint64* pstate;
    int state_size;

    Searcher(Tbls* _tbl, ran_fun _next, int _state_size, bool mwc) {
        tbl = _tbl;
        next = _next;
        state_size = _state_size;
        atts   = new bitboard[1U << 12];
        iters  = new uint64  [1U << 12];

        state  = new uint64  [_state_size];
        pstate = new uint64  [_state_size];
        for(int i = 0; i < _state_size; ++i)
            state[i] = (random() << 32) | random();
        if( mwc ) state[2] = 1;
    }

    bool valid_magic(int pos, bool bsp, uint64 magic) {
        ++total_its;
        for(int j = 0; j < tbl->u[pos][bsp]; ++j) {
            unsigned k = unsigned((magic * tbl->perms[pos][bsp][j]) >> tbl->shamt[pos][bsp]);

            if( iters[k] < total_its ) {
                iters[k] = total_its;
                atts [k] = tbl->moves[pos][bsp][j];
            }
            else if( atts[k] != tbl->moves[pos][bsp][j] ) 
                return false;
        }
        return true;
    }

    void find_magic(int pos, bool bishop, int& cur_it, int min) {
        uint64 magic;
        for(;; ++cur_it) {
            do magic = ran::sparse(next, state);
            while (!magic);

            if( cur_it > min || valid_magic(pos, bishop, magic )) 
                return;
        }
    }

    int find_all(int min) {
        int cur_it = 0;

        for(int i = 0; i < 64; ++i) {
            for(int k: {0, 1}) {
                find_magic(i, k, cur_it, min);
                if( cur_it > min ) 
                    return cur_it;
            }
        }

        return cur_it;
    }

    void skip() {
        do {
            for(int i = 0; i < state_size; ++i)
                pstate[i] = state[i];
        }
        while ( !valid_magic(0, 0, ran::sparse(next, state)) );

        for(int i = 0; i < state_size; ++i)
            state[i] = pstate[i];
    }

    void search() {
        using namespace std;
        for(;;) {
            skip();
            int c = find_all(global_min);
            if( c < global_min ) {
                cout << "seeds: ";
                for(int i = 0; i < state_size; ++i)
                    cout << "s[" << dec << i << "] = 0x" << hex << pstate[i] << "ULL ";
                cout << "\niters = " << dec << c << endl << endl;
                global_min = c;
            } 
        }
    }
};



int main() {
    using namespace std;
    srandom(time(NULL));


    Tbls t{};

    Searcher s0(&t, &ran::xorshift::next, 1, true);

    s0.search();

    return 0;
}
