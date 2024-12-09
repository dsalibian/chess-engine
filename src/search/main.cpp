#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <random>
#include <optional>
#include <mutex>
#include <atomic>
#include <csignal>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>
#include <algorithm>

using std::uint64_t;
using bitboard = std::uint64_t;

namespace timing {
using namespace std::chrono;

auto current_time() { return steady_clock::now(); }

auto elapsed_ms(steady_clock::time_point& t) {
    return duration_cast<milliseconds>(steady_clock::now() - t).count(); }
auto elapsed_sec (steady_clock::time_point& t) { return double(elapsed_ms(t))  / 1'000.0; }
auto elapsed_min (steady_clock::time_point& t) { return elapsed_sec(t) / 60.0; }
auto elapsed_hrs (steady_clock::time_point& t) { return elapsed_min(t) / 60.0; }
auto elapsed_days(steady_clock::time_point& t) { return elapsed_hrs(t) / 24.0; }

} // namespace timing



namespace glbl {
    std::atomic<bool> fexit{false};
}

void handle_sigint(int signal) {
    std::cout << "\nsigint signal " << signal << " received...\n\n";
    glbl::fexit.store(true, std::memory_order_relaxed);
}

bool fexit() {
    return glbl::fexit.load(std::memory_order_relaxed);
}


namespace movegen {

constexpr int dir_delta[8][2] {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

bitboard ray(int sqr, int d, bitboard block) {
    bitboard bb = 0;

    const int df = dir_delta[d][0];
    const int dr = dir_delta[d][1];

    unsigned f = (sqr % 8) + df;
    unsigned r = (sqr / 8) + dr;
    for( ; f <= 7 && r <= 7; f += df, r += dr) {
        bitboard t = 1ULL << (8 * r + f);
        bb |= t;
        if( t & block ) break;
    }
    
    return bb;
}

enum Direction { north,  east, south,  west, neast, nwest, seast, swest, };

enum Files_Ranks {
    a_file  = 0x0101010101010101ULL,
    h_file  = 0x8080808080808080ULL, 
    ah_file = 0x8181818181818181ULL, 

    rank_1  = 0x00000000000000ffULL,
    rank_8  = 0xff00000000000000ULL,
    rank_18 = 0xff000000000000ffULL,
};

bitboard rmask(int sqr, bool bsp) {
    bitboard mask = 0;
    constexpr bitboard r_bounds[4] {~rank_8, ~h_file, ~rank_1, ~a_file};
    constexpr bitboard b_bounds = ~(ah_file | rank_18);
    int offs = bsp ? 4: 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(sqr, dir, 0) & (bsp ? b_bounds : r_bounds[dir]);

    return mask;
}

bitboard satts(int sqr, uint64_t block, bool bsp) {
    bitboard mask = 0;
    int offs = bsp ? 4 : 0;

    for(int dir = north + offs; dir <= west + offs; ++dir)
        mask |= ray(sqr, dir, block);

    return mask;
}

} // namespace movegen



namespace bits {

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

uint64_t next_lperm(uint64_t v) {
    uint64_t t = v | (v - 1);
    return (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctzll(v) + 1));
}

} // namespace bits

__uint128_t nC(unsigned n, unsigned k) {
    return k == 0 || k == n ? 1 : n * nC(n - 1, k - 1) / k;
}

std::string u128_tostring(__uint128_t n, unsigned base) {
    std::string s{""};
    do {
        unsigned k = unsigned(n % base);

        if(base <= 10) s = char('0' + k) + s;
        else if(base <= 36) s = char(k < 10 ? '0' + k : 'a' + k - 10) + s;
        else s = std::to_string(k) + ' ' +  s;

        n /= base;
    } while(n);

    return s;
}

namespace ran {

uint64_t rd64() {
    static std::mutex rand_mutex;
    std::lock_guard<std::mutex> lock(rand_mutex);

    static std::random_device rd;
    return (uint64_t(rd()) << 32) | rd();
}

};

// precaulcuate things that are worth precaulcating
struct Tbls {
    const bitboard**  rmask;
    const bitboard*** perms;
    const bitboard*** moves;

    Tbls() {
        bitboard**  _rmask = new bitboard* [64];
        bitboard*** _perms = new bitboard**[64];
        bitboard*** _moves = new bitboard**[64];

        for(int sqr = 0; sqr < 64; ++sqr) {
            _rmask[sqr] = new bitboard[2];
            _perms[sqr] = new bitboard*[2];
            _moves[sqr] = new bitboard*[2];

            for(bool bsp: {0, 1}) {
                _rmask [sqr][bsp] = movegen::rmask(sqr, bsp);
                unsigned popcnt = __builtin_popcountll(_rmask[sqr][bsp]);
                unsigned u = 1u << popcnt;

                _perms[sqr][bsp] = new bitboard[u];
                _moves[sqr][bsp] = new bitboard[u];

                for(unsigned i = 0; i < u; ++i) {
                    _perms[sqr][bsp][i] = bits::occ_mask(i, popcnt, _rmask[sqr][bsp]);
                    _moves[sqr][bsp][i] = movegen::satts(sqr, _perms[sqr][bsp][i], bsp);
                }
            }
        }

        rmask  = const_cast<const bitboard**>(_rmask);
        perms  = const_cast<const bitboard***>(_perms);
        moves  = const_cast<const bitboard***>(_moves);
    }

    ~Tbls() {
        for(int sqr = 0; sqr < 64; ++sqr) {
            for(bool bsp:{0, 1}) {
                delete[] perms[sqr][bsp];
                delete[] moves[sqr][bsp];
            } 

            delete[] rmask[sqr];
            delete[] perms[sqr];
            delete[] moves[sqr];
        }

        delete[] rmask;
        delete[] perms;
        delete[] moves;
    }
};

struct Checker {
    const Tbls& tbl;
    bitboard* atts;
    unsigned* iters, iter;

    Checker(const Tbls& _tbl): tbl(_tbl) {
        atts = new bitboard[4096];
        iters = new unsigned[4096];
        iter = 0;
    }

    unsigned hash(uint64_t key, uint64_t magic, unsigned shamt) {
        return unsigned((key * magic) >> shamt);
    }

    bool valid_magic(int sqr, bool bsp, uint64_t magic, std::optional<unsigned> shamt = std::nullopt) {
        ++iter;

        int popcnt = __builtin_popcountll(tbl.rmask[sqr][bsp]);
        if(!shamt)
            *shamt = 64 - popcnt;

        for(unsigned i = 0; i < 1u << popcnt; ++i) {
            unsigned k = hash(tbl.perms[sqr][bsp][i], magic, *shamt);

            if(iters[k] != iter) {
                iters[k] = iter;
                atts[k] = tbl.moves[sqr][bsp][i];
            }
            else if(atts[k] != tbl.moves[sqr][bsp][i])
                return false;
        }

        return true;
    }
};

// see how many potential magics your machine can check per second
void perft() {
    const Tbls tbl;
    Checker checker(tbl);

    std::mt19937_64 ran(ran::rd64());

    uint64_t count_magics = 0, count_total = 0, magic;
    auto t = timing::current_time();
    auto t_total = timing::current_time();

    while(!fexit()) {
        magic = ran() | ran();
        
        for(int i = 0; i < 128; ++i)
            if(checker.valid_magic(i % 64, i > 63, magic)) 
                ++count_magics;

        count_total += 128;
        if(timing::elapsed_sec(t) > 1) {
            std::cout << "\raverage so far: " << 
                 int(double(count_total) / timing::elapsed_sec(t_total)) << " magics / sec";

            std::flush(std::cout);
            t = timing::current_time();
        }
    }

    std::cout << std::endl << std::endl;;
}

void searcher(std::atomic<uint64_t>** magics, const Tbls& tbl) {
    std::mt19937_64 ran(ran::rd64());
    Checker checker(tbl);
    static std::mutex cout_mutex;
    uint64_t magic;

    while(!fexit()) {
        magic = ran() | ran();

        for(int sqr = 0; sqr < 64; ++sqr) {
            for(bool bsp: {0, 1}) {
                if(!(magics[sqr][bsp].load(std::memory_order_relaxed)) &&
                        checker.valid_magic(sqr, bsp, magic, 64 - __builtin_popcountll(magic) + 1)) 
                {
                    magics[sqr][bsp].store(magic, std::memory_order_relaxed);

                    std::lock_guard<std::mutex> lock(cout_mutex);
                    unsigned s = __builtin_popcountll(tbl.rmask[sqr][bsp]);
                    std::cout << "found new " << 
                        (bsp ? 'b' : 'r') << char('a' + sqr % 8) << char('1' + sqr / 8) << 
                        " magic. shamt: " << s - 1 << " (" << s << ")\n";
                }
            }
        }
    }
}

void write_pretty(std::atomic<uint64_t>** magics) {
    std::cout << "writing human readable file..." << std::endl;;

    if(!std::filesystem::exists("best_magics"))
        system("touch best_magics");

    const Tbls tbl;
    unsigned sum_ttl = 0, sum_2 = 0;
    std::ofstream out("best_magics");

    out << "zero means no magic with better shift found yet, if it even exists\n\n";

    for(int sqr = 0; sqr < 64; ++sqr) {
        uint64_t rmagic = magics[sqr][0].load(std::memory_order_relaxed);
        uint64_t bmagic = magics[sqr][1].load(std::memory_order_relaxed);
        
        unsigned rshamt = __builtin_popcountll(tbl.rmask[sqr][0]);
        unsigned bshamt = __builtin_popcountll(tbl.rmask[sqr][1]);

        sum_ttl += (1u << rshamt) + (1u << bshamt);
        sum_2 += (1u << (rshamt - (rmagic ? 1 : 0))) + 
            (1u << (bshamt - (bmagic ? 1 : 0)));

        char file = 'a' + char(sqr % 8); 
        char rank = '1' + char(sqr / 8);

        char buffer[1u << 16];

        using ull = unsigned long long;
        std::sprintf(buffer,
                "%c%c - %2d:        rk: 0x%-18llx %-2d (%2d)         bsp: 0x%-18llx %2d (%d)\n", 
                file, rank, sqr, 
                ull(rmagic), rshamt - (rmagic > 0), rshamt, 
                ull(bmagic), bshamt - (bmagic > 0), bshamt);

        out << buffer;
    }

    double sum_ttl_kib = double(sum_ttl * 8) / (1u << 10); 
    double sum_2_kib = double(sum_2 * 8) / (1u << 10); 

    out << "\n\ntable size reduced from " << sum_ttl_kib << "kib to " <<
        sum_2_kib << "kib. (-" << ((sum_ttl_kib - sum_2_kib) / sum_ttl_kib) * 100 << "%)\n";

    out.close();
    std::cout << "done. \n" << std::endl;;
}

void write_bin(std::atomic<uint64_t>** magics) {
    std::cout << "writing to binary file..." << std::endl;;

    if(!std::filesystem::exists("bin"))
        system("touch bin");

    const Tbls tbl;
    uint64_t rmagic, bmagic;

    std::ofstream out("bin", std::ios::binary);
    for(int sqr = 0; sqr < 64; ++sqr) {
        rmagic = magics[sqr][0].load(std::memory_order_relaxed);
        bmagic = magics[sqr][1].load(std::memory_order_relaxed);

        out.write(reinterpret_cast<const char*>(&rmagic), sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(&bmagic), sizeof(uint64_t));
    }

    out.close();
    std::cout << "done. \n" << std::endl;;
}

std::atomic<uint64_t>** init_magics() {
    std::cout << "getting list of magics..." << std::endl;;

    std::atomic<uint64_t>** magics = new std::atomic<uint64_t>*[64]; 
    for(int sqr = 0; sqr < 64; ++sqr) 
        magics[sqr] = new std::atomic<uint64_t>[2]{0, 0};
    
    if(!std::filesystem::exists("bin")) {
        std::cout << "starting fresh search.\n" << std::endl;;
        return magics;
    }

    std::cout << "reading from binary file..." << std::endl;;
    std::ifstream in("bin", std::ios::binary);
    uint64_t rmagic, bmagic;

    for(int sqr = 0; sqr < 64; ++sqr) {
        in.read(reinterpret_cast<char*>(&rmagic), sizeof(uint64_t));
        in.read(reinterpret_cast<char*>(&bmagic), sizeof(uint64_t));

        magics[sqr][0].store(rmagic, std::memory_order_relaxed);
        magics[sqr][1].store(bmagic, std::memory_order_relaxed);
    }

    in.close();
    std::cout << "done. \n" << std::endl;;
    return magics;
}


void go(int thread_count) {
    std::atomic<uint64_t>** magics = init_magics();
    const Tbls tbl;

    std::cout << "starting search with " << thread_count << " threads...\n\n" << std::endl;

    std::thread** threads = new std::thread*[thread_count];
    for(int i = 0; i < thread_count; ++i) 
        threads[i] = new std::thread(&searcher, magics, std::ref(tbl));

    for(int i = 0; i < thread_count; ++i) 
        threads[i]->join();
    
    std::cout << "all threads joined succesfully\n" << std::endl;

    write_bin(magics);
    write_pretty(magics);
}

bool first_time_nigga = true;
uint64_t ran_nbit(int bitcount) {
    static std::mt19937_64 mt(9);
    static std::uniform_int_distribution ran(0, 64);
    static uint64_t* bit = new uint64_t[64];

    if(first_time_nigga) {
        first_time_nigga = false;
        for(int i = 0; i < 64; ++i)
            bit[i] = 1ull << i;
    }

    uint64_t n = 0;
    int index;
    for(int i = 0; i < bitcount; ++i) {
        do index = ran(mt);
        while(n & bit[index]);

        n |= bit[index];
    }

    return n;
}


void find_all_magics() {
    const Tbls tbl;
    Checker checker(tbl);

    auto first_perm = [](int popcnt) {
        uint64_t t = 0;
        for(int i = 0; i < popcnt; ++i)
            t |= 1ull << i;
        return t;
    };

    system("touch allmagics");
    std::ofstream out("allmagics", std::ios::binary);

    uint64_t count = 0;
    for(int popcnt = 1; popcnt < 13 && !fexit(); ++popcnt) {
        auto t = timing::current_time();
        uint64_t cur = first_perm(popcnt);

        __uint128_t u = nC(64, popcnt);    
        for(__uint128_t i = 0; i < u && !fexit(); ++i, cur = bits::next_lperm(cur)) {
            for(int j = 0; j < 128; ++j) {
                if(checker.valid_magic(j % 64, j > 63, cur)) {
                    out.write(reinterpret_cast<const char*>(&cur), sizeof(uint64_t));
                    ++count;
                    break;
                }
            }

            if(timing::elapsed_ms(t) > 1000) {
                t = timing::current_time();
                std::cout << "\r                            " <<
                    '\r' << double(i) / double(u) * 100 << " " << count;
                std::flush(std::cout);
            }
        }
        
        std::cout << "\r                                " <<
            "\rfinished popcnt = " << popcnt << '\n';
    }
    out.close();

    if(!fexit())
        std::cout << "SEARCH COMPLETE\n\n";
    else {
        std::cout << "resetting search\n";
        system("rm allmagics");
    }
}

int main() {
    std::signal(SIGINT, handle_sigint);

    find_all_magics();
    
    std::cout << "exiting...\n";
    return 0;
}
