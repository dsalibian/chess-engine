#include "tbls.h"
#include "ran.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>
#include <fstream>
#include <filesystem>
#include <csignal>
#include <thread>


// https://graphics.stanford.edu/%7Eseander/bithacks.html#NextBitPermutation
struct LPerm {
    unsigned popcnt;
    uint64 max, it = 0;
    uint64 cur;
    
    LPerm() {}

    void init(unsigned _popcnt) {
        if( _popcnt > 26 ) std::cout << "WARNING: overflow in nC()\n\n";
        popcnt = _popcnt;
        max = nC(64, popcnt);

        cur = 0;
        for(unsigned i = 0; i < popcnt; ++i)
            cur |= 1ULL << i;
    }

    uint64 next() {
        uint64 t = cur | (cur - 1);
        cur = (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctzll(cur) + 1));
        return cur;
    }

    // good for n = 64, k <= 26
    static constexpr uint64 nC(unsigned n, unsigned k) {
        if( !k ) return 1;
        return nC(n - 1, k - 1) * n / k;
    }
};

const Tbls* const glbl_tbl = new Tbls{};
struct Magic_Checker {
    static const Tbls* tbl;
    bitboard* atts;
    unsigned* iters, total_its = 0;

    Magic_Checker() {
        tbl = glbl_tbl;
        atts  = new bitboard[4096];
        iters = new unsigned[4096];
    }

    bool valid_magic(int pos, bool bsp, uint64 magic, unsigned shamt) {
        ++total_its;
        for(int j = 0; j < tbl->u[pos][bsp]; ++j) {
            unsigned k = unsigned((magic * tbl->perms[pos][bsp][j]) >> shamt);

            if( iters[k] != total_its ) {
                iters[k] = total_its;
                atts [k] = tbl->moves[pos][bsp][j];
            }
            else if( atts[k] != tbl->moves[pos][bsp][j] ) 
                return false;
        }
        return true;
    }

    unsigned best_shamt(int pos, bool bsp, uint64 magic) {
        for(unsigned i = 63; i > 0; --i) 
            if(valid_magic(pos, bsp, magic, i))
                return i;

        abort(); // blow it all up
    }
};

volatile sig_atomic_t fexit = 0;
void handleSigint(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nstop signal received" << std::endl;
        fexit = 1;
    }
}

uint64*** glbl_best_magics;

template<typename Seq>
struct Searcher {
    Magic_Checker mc;
    Seq seq;
    uint64** best_magic;
    unsigned** best_shamt;
    unsigned max_iters;
    unsigned id; 

    Searcher(unsigned _max_iters, unsigned _id) {
        mc = Magic_Checker{};

        best_magic = new uint64*[64];
        best_shamt = new unsigned*[64];
        for(int i = 0; i < 64; ++i) {
            best_magic[i] = new uint64[2];
            best_shamt[i] = new unsigned[2];
        }

        max_iters = _max_iters;
        id = _id;
        glbl_best_magics[_id] = best_magic;     

        if(std::filesystem::file_size("best_magics"))
            read();
        else
            set_best_magic();
        set_best_shamt();
    }

    void set_best_shamt() {
        for(int i = 0; i < 64; ++i) {
            best_shamt[i][0] = mc.best_shamt(i, 0, best_magic[i][0]);
            best_shamt[i][1] = mc.best_shamt(i, 1, best_magic[i][1]);
        }
    }

    void set_best_magic() {
        unsigned t = max_iters;
        max_iters = UINT32_MAX;
        for(int i = 0; i < 64; ++i) {
            best_shamt[i][0] = mc.tbl->shamt[i][0];
            best_shamt[i][1] = mc.tbl->shamt[i][1];

            best_magic[i][0] = next_magic(i, 0);
            best_magic[i][1] = next_magic(i, 1);
        }
        max_iters = t;
    }

    void read() {
        std::ifstream in("best_magics");
        for(int i = 0; i < 64; ++i) {
            in >> std::hex >> best_magic[i][0] >> best_magic[i][1];
            if(!best_magic[i][0] || !best_magic[i][1]) {
                std::cout << "bad magic, exiting...\n";
                std::quick_exit(1);
            }
        }
        in.close();
    }

    uint64 next_magic(int pos, bool bsp) {
        uint64 magic;
        for(unsigned i = 0; i < max_iters; ++i) {
            do magic = seq.next();
            while (!magic);

            if( fexit || mc.valid_magic(pos, bsp, magic, best_shamt[pos][bsp]) )
                return magic;
        }
        return 0;
    }

    void search() {
        for(int i = 0; ; ++i) {
            for(int j: {0, 1}) {
                uint64 t = next_magic(i % 64, j);

                if(fexit) 
                    goto end_search;

                if(t) {
                    best_magic[i % 64][j] = t;
                    best_shamt[i % 64][j] = mc.best_shamt(i, j, t);
                }
            }
        }
        end_search:;
        std::cout << "thread " << id << " exiting...\n";
    } 
};

template<typename Seq>
void start_search(int thread_count) {
    using std::thread;

    glbl_best_magics = new uint64**[thread_count];

    if(!std::filesystem::exists("best_magics"))
        system("touch best_magics");


    thread* threads = (thread*) malloc(thread_count * sizeof(thread));
    for(int i = 0; i < thread_count; ++i) 
        threads[i] = thread(&Searcher<Seq>::search, Searcher<Seq>(10, i));
    
    for(int i = 0; i < thread_count; ++i)
        threads[i].join();

    Magic_Checker mc{};
    for(int i = 1; i < thread_count; ++i)
        for(int j = 0; j < 64; ++j)
            for(int k = 0; k < 2; ++k)
                if( mc.best_shamt(j, k, glbl_best_magics[i][j][k]) > 
                        mc.best_shamt(j, k, glbl_best_magics[0][j][k]))
                    glbl_best_magics[0][j][k] = glbl_best_magics[i][j][k];


     
    std::ofstream out("best_magics");
    for(int i = 0; i < 64; ++i)
        out << "0x" << std::hex << glbl_best_magics[0][i][0] << " 0x" << glbl_best_magics[0][i][1] << std::endl;
    out.close();
}

int main() {
    using namespace std;
    signal(SIGINT, handleSigint);

    start_search<ran::xorshiro>(1);

    return 0;
}
