#include "tbls.h"
#include "ran.h"
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> fexit{false};
void handleSigint(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nstop signal received" << std::endl;
        fexit = true;
    }
}

struct Checker {
    Tbls* tbl;
    bitboard* atts;
    unsigned* iters, total_iters;

    Checker(Tbls* _tbl) {
        tbl = _tbl;
        atts  = new bitboard[1U << 12];
        iters = new unsigned[1U << 12]{1};
        total_iters = 0;
    }

    ~Checker() {
        delete[] atts;
        delete[] iters;
    }

    bool valid_magic(int pos, bool bsp, uint64 magic, unsigned shamt) {
        ++total_iters;
        for(int j = 0; j < tbl->u[pos][bsp]; ++j) {
            unsigned k = unsigned((magic * tbl->perms[pos][bsp][j]) >> shamt);

            if(iters[k] != total_iters) {
                iters[k] = total_iters;
                atts[k]  = tbl->moves[pos][bsp][j];
            }
            else if(atts[k] != tbl->moves[pos][bsp][j]) 
                return false;
        }
        return true;
    }

    unsigned best_shamt(int pos, bool bsp, uint64 magic) {
        for(unsigned i = 63; i > 0; --i)
            if(valid_magic(pos, bsp, magic, i))
                return i;
        abort();
    }

    template<typename Seq>
    uint64 sparse(Seq& seq) {
        return seq.next() & seq.next() & seq.next();
    }

    template<typename Seq>
        uint64 next_magic(int pos, bool bsp, unsigned shamt, Seq& seq, bool ran, unsigned max_iters) {
            uint64 magic;

            for(unsigned i = 0; i < max_iters; ++i) {
                do magic = ran ? sparse<Seq>(seq) : seq.next();
                while(!magic);

                if(fexit)
                    return 0;

                if(valid_magic(pos, bsp, magic, shamt))
                    return magic;
            }

            return 0;
        }
};

struct LPerm {
    unsigned popcnt;
    uint64 max_iters, cur_iter;
    uint64 cur;
    
    LPerm(unsigned _popcnt) {
        popcnt = _popcnt;
        max_iters = nC(64, popcnt);
        cur_iter = 0;

        cur = 0;
        for(unsigned i = 0; i < popcnt; ++i)
            cur |= 1ULL << i;
    }

    // https://graphics.stanford.edu/%7Eseander/bithacks.html#NextBitPermutation
    uint64 next() {
        ++cur_iter;
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

template<typename Ran>
void searcher(Tbls* tbl, uint64** best_magic, unsigned** best_shamt, int max_iters) {
    Checker checker(tbl);
    Ran ran; 

    for(unsigned i = 0; ; ++i) {
        for(int j = 0; j < 2; ++j) {
            uint64_t magic = checker.next_magic<Ran>(i % 64, j, best_shamt[i % 64][j], ran, 1, max_iters);

            if(fexit) 
                return;

            if(magic) {
                best_magic[i % 64][j] = magic;
                best_shamt[i % 64][j]++;
            }
        }
    }
}

void get_best(uint64*** magics, unsigned*** shamts, int len) {
    for(int i = 1; i < len; ++i)
        for(int j = 0; j < 64; ++j)
            for(int k = 0; k < 2; ++k)
                if(shamts[i][j][k] > shamts[0][j][k])
                    shamts[0][j][k] = shamts[i][j][k], magics[0][j][k] = magics[i][j][k];
}

void read(uint64** magics, unsigned** shamts) {
    std::ifstream in("best_magics");
    using std::hex, std::dec;
    for(int i = 0; i < 64; ++i) {
        in >> hex >> magics[i][0] >> dec >> shamts[i][0];
        in >> hex >> magics[i][1] >> dec >> shamts[i][1];
    }
    in.close();
}

void write(uint64** magics, unsigned** shamts) {
    if(!std::filesystem::exists("best_magics"))
        system("touch best_magics");

    using std::hex, std::dec;
    std::ofstream out("best_magics");
    for(int i = 0; i < 64; ++i) {
        out << hex <<  "0x" << magics[i][0] << " " << dec << shamts[i][0];
        out << hex << " 0x" << magics[i][1] << " " << dec << shamts[i][1];
        out << std::endl;
    }
    out.close();
}

std::pair<uint64**, unsigned**> find_magics() {
    Tbls tbl;
    Checker checker(&tbl);
    ran::mwc ran;

    uint64** best_magics = new uint64*[64];
    unsigned** best_shamt = new unsigned*[64];
    for(int i = 0; i < 64; ++i) {
        best_magics[i] = new uint64[2];
        best_shamt[i] = new unsigned[2];
        for(int j: {0, 1}) {
            best_magics[i][j] = checker.next_magic<ran::mwc>(i, j, tbl.shamt[i][j], ran, 1, UINT32_MAX); 
            best_shamt[i][j]  = checker.best_shamt(i, j, best_magics[i][j]); 
        }
    }

    return std::make_pair(best_magics, best_shamt);
}

std::pair<uint64***, unsigned***> init_bests(int thread_count) {
    uint64*** best_magic = new uint64**[thread_count];
    unsigned*** best_shamt = new unsigned**[thread_count];

    for(int i = 0; i < thread_count; ++i) {
        best_magic[i] = new uint64*[64];
        best_shamt[i] = new unsigned*[64];
        for(int j = 0; j < 64; ++j) {
            best_magic[i][j] = new uint64[2];
            best_shamt[i][j] = new unsigned[2];
        }
    } 

    if(std::filesystem::exists("best_magics"))
        read(best_magic[0], best_shamt[0]);
    else {
        std::pair<uint64**, unsigned**> bests = find_magics();
        best_magic[0] = bests.first;
        best_shamt[0] = bests.second;
    }

    for(int i = 1; i < thread_count; ++i) {
        for(int j = 0; j < 64; ++j) {
            for(int k = 0; k < 2; ++k) {
                best_magic[i][j][k] = best_magic[0][j][k];
                best_shamt[i][j][k] = best_shamt[0][j][k];
            }
        }
    }    

    return std::make_pair(best_magic, best_shamt);
}

void show_best() {
    uint64** magics = new uint64*[64];
    unsigned** shamts = new unsigned*[64];
    for(int i = 0; i < 64; ++i) {
        magics[i] = new uint64[2];
        shamts[i] = new unsigned[2];
    }
    Tbls tbl;
    read(magics, shamts);
    
    std::cout << "bits needed for tbl:\n";
    for(int i = 0; i < 64; ++i) {
        unsigned sr0 = 64 - shamts[i][0], sr1 = 64 - tbl.shamt[i][0];
        unsigned sb0 = 64 - shamts[i][1], sb1 = 64 - tbl.shamt[i][1];
        std::cout << i << ":     " << (sr0 < sr1 ? "!" : " ") << "rook: " << sr0 << " (" << sr1 << ")";
        std::cout << "      " << (sb0 < sb1 ? "!" : " ") << "bishop: " << sb0 << " (" << sb1 << ")\n";
    }
}

template<typename Ran>
void go_ran(int thread_count, int max_iters) {
    std::pair<uint64***, unsigned***> bests = init_bests(thread_count);
    Tbls tbl;

    using std::thread;
    thread* threads = new thread[thread_count];
    for(int i = 0; i < thread_count; ++i)
        threads[i] = thread(&searcher<Ran>, &tbl, bests.first[i], bests.second[i], max_iters);
    
    for(int i = 0; i < thread_count; ++i) 
        threads[i].join();
    
    std::cout << "stopped threads, writing data...\n";
    get_best(bests.first, bests.second, thread_count); 
    write(bests.first[0], bests.second[0]);
}

void go_exhaustive() {
    std::pair<uint64***, unsigned***> bests = init_bests(1);
    Tbls tbl;
    Checker c(&tbl);
    for(int i = 1; i < 27 && !fexit; ++i) {
        LPerm p(i);
        LPerm q(64 - i);
        do {
            for(int j = 0; j < 64; ++j) {
                for(int k = 0; k < 2; ++k) {
                    if(c.valid_magic(j, k, p.cur, bests.second[0][j][k] + 1)) {
                        bests.first[0][j][k] = p.cur;
                        bests.second[0][j][k]++;
                    } 

                    if(c.valid_magic(j, k, q.cur, bests.second[0][j][k] + 1)) {
                        bests.first[0][j][k] = q.cur;
                        bests.second[0][j][k]++;
                    } 
                }
            }

            if( p.cur_iter % 1000 == 0)
                std::cout << '\r' << i << ": " << (double(p.cur_iter) / p.max_iters) * 100;

            p.next();
            q.next();
        } while(p.cur_iter < p.max_iters && !fexit);

        std::cout << std::endl;
    }
     
    write(bests.first[0], bests.second[0]);
}

int main() {
    std::signal(SIGINT, handleSigint);

    go_exhaustive();
    // show_best();

    return 0;
}
