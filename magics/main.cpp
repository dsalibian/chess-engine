#include "../src/misc.h"
#include "../src/movegen.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

struct splitmix {
    uint64 state;
    uint64 next() {
        uint64_t r = (state += 0x9e3779b97f4a7c15ULL);
        r = (r ^ (r >> 30)) * 0xbf58476d1ce4e5b9ULL;
        r = (r ^ (r >> 27)) * 0x94d049bb133111ebULL;
        return state;
    }
};

enum Rk_Bsp {rook, bishop};
struct Tbls {
    bitboard**  rmask;
    int**       popcnt;
    int**       shamt;
    int**       u;
    bitboard*** perms;
    bitboard*** moves;

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
                        perms[s][t][i] = occ_mask(i, popcnt[s][t], rmask[s][t]);
                        moves[s][t][i] = movegen::satts(s, perms[s][t][i], t);
                    }
                }
            }
        }
    }
};

std::atomic<int> global_min{ INT32_MAX };

struct Searcher {
    Tbls*        tbls;
    xorshiro     ran;
    uint64*      iters;
    bitboard*    atts;
    uint64       tot_its;
    
    Searcher(Tbls* _tbls, uint64 s0, uint64 s1, int n) {
        tbls     = _tbls;
        iters   = new uint64[4096]{0};
        atts    = new bitboard[4096];
        tot_its = 0;

        uint64 t = std::chrono::steady_clock::now().time_since_epoch().count();
        splitmix sm{0x2545F4914F6CDD1DULL * t + (ah_file ^ rank_7)};
        for(int i = 0; i < 999; ++i) sm.next();
        ran.state[0] = s0;
        ran.state[1] = s1;
        for(int i = 0; i < n; ++i) ran.jump();
    }

    bool valid_magic(int pos, bool bsp, uint64 magic) {
        ++tot_its;
        for(int j = 0; j < tbls->u[pos][bsp]; ++j) {
            unsigned k = unsigned((magic * tbls->perms[pos][bsp][j]) >> tbls->shamt[pos][bsp]);

            if( iters[k] < tot_its ) {
                iters[k] = tot_its;
                atts [k] = tbls->moves[pos][bsp][j];
            }
            else if( atts[k] != tbls->moves[pos][bsp][j] ) 
                return false;
        }
        return true;
    }
    
    void find_magic(int pos, bool bishop, int& iter, const int min) {
        uint64 magic;
        for(;; ++iter) {
            do magic = ran.sparse();
            while (!magic);

            if( iter > min || valid_magic(pos, bishop, magic )) 
                return;
        }
    }

    int find_all(int min) {
        int iter = 0;

        for(int i = 0; i < 64; ++i) {
            for(int k: {0, 1}) {
                find_magic(i, k, iter, min);
                if( iter > min ) return iter;
            }
        }

        return iter;
    }

    void jnext_ra1() {
        uint64 pstate0;
        uint64 pstate1;
        do pstate0 = ran.state[0], pstate1 = ran.state[1];
        while ( !valid_magic(0, 0, ran.sparse()) );
        ran.state[0] = pstate0;
        ran.state[1] = pstate1;
    }

    void search() {
        for(;;) {
            jnext_ra1();
            uint64 s0 = ran.state[0], s1 = ran.state[1];
            int t = find_all(global_min);
            if( t < global_min ) {
                std::cout << "s[0]: 0x" << std::hex << s0 << "ULL  s[1]: 0x" << s1 << std::dec << "ULL  " << t << " iters\n";
                global_min = t;
            }
        }
    }
};

void start_search(int thread_count) {
    using std::vector, std::thread;

    Tbls tbl{};
    vector<thread> v;

    uint64 t = std::chrono::steady_clock::now().time_since_epoch().count();
    splitmix sm{0x2545F4914F6CDD1DULL * t + (ah_file ^ rank_7)};
    for(int i = 0; i < 999; ++i) sm.next();
    uint64 s0 = sm.next();
    uint64 s1 = sm.next();

    for(int i = 0; i < thread_count; ++i) 
        v.push_back(thread(&Searcher::search, Searcher(&tbl, s0, s1, i)));

    for(int i = 0; i < thread_count; ++i) 
        v[i].join();
}

int main() {
    using namespace std;
    using namespace chrono;

    start_search(10);


    


    return 0;
}
