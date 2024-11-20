#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <chrono>
#include <initializer_list>
#include <atomic>
#include <mutex>
#include <csignal>
#include <shared_mutex>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string>

using std::uint64_t;
using bitboard = std::uint64_t;




namespace glbl {
    std::mutex cout_mutex;
    std::atomic<bool> fexit{false};
}

void handle_sigint(int signal) {
    std::cout << "\nsigint signal " << signal << " received...\n\n";
    glbl::fexit.store(true, std::memory_order_relaxed);
}

bool fexit() {
    return glbl::fexit.load(std::memory_order_relaxed);
}




namespace timing {
using namespace std::chrono;

auto current_time() {
    return steady_clock::now();
}

auto elapsed(steady_clock::time_point t0) {
    return duration_cast<milliseconds>(steady_clock::now() - t0).count();
}

} // namespace timing




namespace movegen {

enum Files_Ranks {
    a_file  = 0x0101010101010101ULL,
    h_file  = 0x8080808080808080ULL, 
    ah_file = 0x8181818181818181ULL, 

    rank_1  = 0x00000000000000ffULL,
    rank_8  = 0xff00000000000000ULL,
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

bool get_bit(bitboard bb, int k) {
    return (bb >> k) & 1;
}

void pop_lsb(bitboard& bb) {
    bb &= bb - 1; 
}

void print_bb(bitboard bb) {
    using std::cout;

    for(int r = 7; r >= 0; --r) {
        cout << ' ' << r + 1 << "   ";
        for(int f = 0; f <= 7; ++f) 
            cout << (get_bit(bb, 8 * r + f) ? "1 " : ". ");
        cout << '\n';
    }
    cout << "\n     a b c d e f g h\n\n\n";
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

constexpr __uint128_t init_u128(uint64_t hi, uint64_t lo) {
    return (__uint128_t(hi) << 64) | lo;
}

std::string u128_tostring(__uint128_t n) {
    if(!n)
        return "0";

    std::string s{""};
    while(n) {
        s = char('0' + (n % 10)) + s;
        n /= 10;
    }

    return s;
}

std::string u128_tostring16(__uint128_t n) {
    if(!n)
        return "0"; 

    std::string str{""};
    int i = 0;
    while(n) {
        if(i++ == 16) 
            str = ' ' + str;

        int k = n % 16;
        str = char(k > 9 ? 'a' + (k - 10) : '0' + k) + str;

        n /= 16;
    }

    return str;
}

} // namespace bits



namespace seqs{

namespace ran {
    
// https://prng.di.unimi.it/splitmix64.c
struct splitmix {
    static constexpr int size = 1;
    uint64_t state[size];

    splitmix() { 
        *state = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
        for(int i = 0; i < 0x999999; ++i) 
            *state = this->next();
    }

    uint64_t next() {
        uint64_t t = (*state += 0x9e3779b97f4a7c15ULL);
        t = (t ^ (t >> 30)) * 0xbf58476d1ce4e5b9ULL;
        t = (t ^ (t >> 27)) * 0x94d049bb133111ebULL;
        return t ^ (t >> 31);
    }
};

void seed(uint64_t* state, int size) {
    splitmix m;
    for(int i = 0; i < size; ++i) 
        state[i] = m.next();
}

// https://prng.di.unimi.it/xoshiro512plusplus.c
struct xorshiro {
    static constexpr int size = 8;
    uint64_t state[size];

    xorshiro() { 
        seed(state, size); 
    };

    uint64_t next() {
        uint64_t result = xorshiro_rotl(state[0] + state[2], 17) + state[2];
        uint64_t t = state[1] << 11;

        state[2] ^= state[0];
        state[5] ^= state[1];
        state[1] ^= state[2];
        state[7] ^= state[3];
        state[3] ^= state[4];
        state[4] ^= state[5];
        state[0] ^= state[6];
        state[6] ^= state[7];
        state[6] ^= t;
        state[7] = xorshiro_rotl(state[7], 21);

        return result;
    }

    uint64_t xorshiro_rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
};

// https://github.com/imneme/pcg-cpp/blob/master/include/pcg_random.hpp
// https://github.com/lemire/testingRNG/blob/master/source/pcg64.h
struct pcg {
    static constexpr int size = 2;
    static constexpr __uint128_t MULT = bits::init_u128(0x2360ed051fc65da4ULL, 0x4385df649fccf645ULL); 
    static constexpr __uint128_t INC  = bits::init_u128(0x5851f42d4c957f2dULL, 0x14057b7ef767814fULL); 
    uint64_t state[size];

    pcg() { 
        seed(state, size); 
    }

    uint64_t next() {
        enum {hi, lo};

        __uint128_t s = bits::init_u128(state[hi], state[lo]);
        __uint128_t t = s * MULT + INC;

        state[hi] = uint64_t(t >> 64);
        state[lo] = uint64_t(t);

        return rotr(state[hi] ^ state[lo], unsigned(t >> 122));
    }

    uint64_t rotr(uint64_t x, unsigned k) {
        return (x >> k) | (x << ((-k) & 63));
    }

};

} // namespace ran

struct LPerm {
    unsigned popcnt;
    uint64_t state;
    __uint128_t cur_iter, max_iter;

    LPerm(unsigned _popcnt) { init(_popcnt); }

    void init(unsigned _popcnt) {
        popcnt = _popcnt;
        cur_iter = 0;
        max_iter = nC(64, popcnt);

        state = 0;
        for(unsigned i = 0; i < popcnt; ++i)
            state |= 1ULL << i;
    }

    // https://graphics.stanford.edu/%7Eseander/bithacks.html#NextBitPermutation
    uint64_t next() {
        ++cur_iter;
        uint64_t t = state | (state - 1);
        state = (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctzll(state) + 1));
        return state;
    }

    long double progress() {
        return (long double)((long double)cur_iter / (long double)max_iter * 100); // make the compiler shutup
    }

    static constexpr __uint128_t nC(unsigned n, unsigned k) {
        if( !k ) return 1;
        return nC(n - 1, k - 1) * n / k;
    }
};

} // namespace seqs



struct Tbls {
    const bitboard**  rmask;
    const unsigned**  popcnt;
    const unsigned**  shamt;
    const unsigned**  u;
    const bitboard*** perms;
    const bitboard*** moves;

    Tbls() {
        bitboard** _rmask  = new bitboard*[64];
        unsigned** _popcnt = new unsigned*[64];
        unsigned** _u      = new unsigned*[64];
        unsigned** _shamt  = new unsigned*[64];
        bitboard*** _perms = new bitboard**[64];
        bitboard*** _moves = new bitboard**[64];

        for(int sqr = 0; sqr < 64; ++sqr) {
            _rmask[sqr]  = new bitboard[2];
            _popcnt[sqr] = new unsigned[2];
            _u[sqr]      = new unsigned[2];
            _shamt[sqr]  = new unsigned[2];
            _perms[sqr]  = new bitboard*[2];
            _moves[sqr]  = new bitboard*[2];

            for(int bsp: {0, 1}) {
                _rmask [sqr][bsp] = movegen::rmask(sqr, bsp);
                _popcnt[sqr][bsp] = __builtin_popcountll(_rmask[sqr][bsp]);
                _u     [sqr][bsp] = 1U << _popcnt[sqr][bsp];
                _shamt [sqr][bsp] = 64  - _popcnt[sqr][bsp];
                _perms [sqr][bsp] = new bitboard[_u[sqr][bsp]];
                _moves [sqr][bsp] = new bitboard[_u[sqr][bsp]];
            }

            for(int bsp: {0, 1}) {
                for(unsigned i = 0; i < _u[sqr][bsp]; ++i) {
                    _perms[sqr][bsp][i] = bits::occ_mask(i, _popcnt[sqr][bsp], _rmask[sqr][bsp]);
                    _moves[sqr][bsp][i] = movegen::satts(sqr, _perms[sqr][bsp][i], bsp);
                }
            }
        }

        rmask  = const_cast<const bitboard**>(_rmask);
        popcnt = const_cast<const unsigned**>(_popcnt);
        shamt  = const_cast<const unsigned**>(_shamt);
        u      = const_cast<const unsigned**>(_u);
        perms  = const_cast<const bitboard***>(_perms);
        moves  = const_cast<const bitboard***>(_moves);
    }

    ~Tbls() {
        for(int sqr = 0; sqr < 64; ++sqr) {
            delete[] rmask[sqr];
            delete[] popcnt[sqr];
            delete[] u[sqr];
            delete[] shamt[sqr];
            
            for(int bsp:{0, 1}) {
                delete[] perms[sqr][bsp];
                delete[] moves[sqr][bsp];
            } 

            delete[] perms[sqr];
            delete[] moves[sqr];
        }

        delete[] rmask;
        delete[] popcnt;
        delete[] u;
        delete[] shamt;
        delete[] perms;
        delete[] moves;
    }
};




namespace magics {

struct Magic {
    std::atomic<uint64_t> key;
    std::atomic<unsigned> shamt;

    Magic() {}
    Magic(uint64_t _key, unsigned _shamt): key(_key), shamt(_shamt) {}

    void update(uint64_t _key, unsigned _shamt) {
        key.store(_key, std::memory_order_relaxed);
        shamt.store(_shamt, std::memory_order_relaxed);
    }

    uint64_t get_key() { 
        return key.load(std::memory_order_relaxed);
    }

    unsigned get_shamt() { 
        return shamt.load(std::memory_order_relaxed);
    }

};

struct MChecker {
    const Tbls& tbl;
    bitboard* atts;
    uint64_t* iters, iter;

    MChecker(const Tbls& _tbl): tbl(_tbl) {
        atts  = new bitboard[1U << 12];
        iters = new uint64_t[1U << 12];
        iter = 0;
    }

    ~MChecker() {
        delete[] atts;
        delete[] iters;
    }

    bool valid_magic(int sqr, bool bsp, uint64_t magic, std::optional<unsigned> shamt = std::nullopt) {
        ++iter;
        *shamt = shamt ? *shamt : tbl.shamt[sqr][bsp]; 

        for(unsigned i = 0; i < tbl.u[sqr][bsp]; ++i) {
            unsigned k = unsigned((magic * tbl.perms[sqr][bsp][i]) >> *shamt);

            if(iters[k] != iter) {
                iters[k] = iter;
                atts[k] = tbl.moves[sqr][bsp][i];
            }
            else if(atts[k] != tbl.moves[sqr][bsp][i])
                return false;
            
        }
        return true;
    }

    unsigned best_shamt(int sqr, bool bsp, uint64_t magic) {
        for(unsigned i = 61; i >= tbl.shamt[sqr][bsp]; --i)
            if(valid_magic(sqr, bsp, magic, i))
                return i;

        return 0; 
    }
};

Magic** generate_best() {
    auto t = timing::current_time();
    std::cout << "generating new magics..\n";

    Magic** best = new Magic*[64];
    Tbls tbl;
    MChecker mc(tbl);
    seqs::ran::pcg ran;
    uint64_t magic;
    unsigned its = 0;

    for(int sqr = 0; sqr < 64; ++sqr) {
        best[sqr] = new Magic[2];
        for(int bsp: {0, 1}) {
            do {
                do magic = ran.next() & ran.next() & ran.next(), ++its;
                while(!magic);
            } while(!mc.valid_magic(sqr, bsp, magic));

            best[sqr][bsp].update(magic, tbl.shamt[sqr][bsp]); 
        }
    }

    std::cout << "generated all magics in " << its << " iterations (" 
        << timing::elapsed(t) << "ms)\n"<< std::endl;
    return best;
}

Magic** init_best() {
    if(!std::filesystem::exists("dat"))
        return generate_best();

    auto t = timing::current_time();
    std::cout << "reading magics..\n";

    Magic** best = new Magic*[64];
    std::ifstream in("dat", std::ios::binary);
    uint64_t magic;
    unsigned shamt;

    for(int sqr = 0; sqr < 64; ++sqr) {
        best[sqr] = new Magic[2];
        for(int bsp: {0, 1}) {
            in.read(reinterpret_cast<char*>(&magic),  sizeof(uint64_t));
            in.read(reinterpret_cast<char*>(&shamt),  sizeof(unsigned));

            best[sqr][bsp].update(magic, shamt);
        }
    }
    
    in.close();
    std::cout << "read magics succesfully " << "(" << timing::elapsed(t) << "ms)\n"<< std::endl;
    return best;
}

void write_best_bin(Magic** best) {
    if(!std::filesystem::exists("dat"))
        system("touch dat");

    std::ofstream out("dat", std::ios::binary);

    for(int sqr = 0; sqr < 64; ++sqr) {
        for(int bsp: {0, 1}) {
            uint64_t magic = best[sqr][bsp].get_key();
            unsigned shamt = best[sqr][bsp].get_shamt();

            out.write(reinterpret_cast<const char*>(&magic),  sizeof(uint64_t));
            out.write(reinterpret_cast<const char*>(&shamt),  sizeof(unsigned));
        }
    }

    out.close();
}

void write_best_pretty(Magic** best) {
    if(!std::filesystem::exists("best_magics"))
        system("touch best_magics");

    Tbls tbl;
    std::ofstream out("best_magics");

    out << "square:       key:                     bits used: (original)\n\n";
    for(int sqr = 0; sqr < 64; ++sqr) {
        uint64_t magic_rk  = best[sqr][0].get_key();         
        uint64_t magic_bsp = best[sqr][1].get_key();         
        unsigned shamt_rk  = best[sqr][0].get_shamt();
        unsigned shamt_bsp = best[sqr][1].get_shamt();

        char file = 'a' + char(sqr % 8); 
        char rank = '1' + char(sqr / 8);
        bool b0 = 64 - shamt_rk  < tbl.popcnt[sqr][0];
        bool b1 = 64 - shamt_bsp < tbl.popcnt[sqr][1];
        bool b2 = 64 - shamt_rk  < tbl.popcnt[sqr][0] - 1;
        bool b3 = 64 - shamt_bsp < tbl.popcnt[sqr][1] - 1;

        char buffer[1U << 16];

        using ull = unsigned long long;
        std::sprintf(
                buffer, 
                "%c%c:         %c%crk: 0x%-18llx %2d (%2d)         %c%cbsp: 0x%-18llx %2d (%d)",
                file, rank,
                b2 ? '!' : ' ', b0 ? '!' : ' ', 
                (ull)magic_rk,  64 - shamt_rk,  tbl.popcnt[sqr][0],
                b3 ? '!' : ' ', b1 ? '!' : ' ', 
                (ull)magic_bsp, 64 - shamt_bsp, tbl.popcnt[sqr][1]);

        out << buffer << '\n';  
    }

    out.close();
}

void write_best(Magic** best) {
    std::cout << "writing data...\n";
    auto t = timing::current_time();

    write_best_bin(best);
    write_best_pretty(best);

    std::cout << "wrote data succesfully (" << timing::elapsed(t) << "ms)\n" << std::endl;
}

} // namespace magics



struct Searcher {
    magics::MChecker mc;
    magics::Magic** best;

    Searcher(const Tbls& _tbl, magics::Magic** _best): mc(_tbl), best(_best) {}

    void esearch(int start, __uint128_t offs, uint64_t start_state) {
        seqs::LPerm lp(start);
        if(offs) {
            lp.cur_iter = offs;
            lp.state = start_state; // pick up where we left off
        }

        do {

            // add mutex locks for cout when this gets multithreaded
            std::cout << "searching exhaustively with n = " << lp.popcnt << '\n';

            while(lp.cur_iter != lp.max_iter && !fexit()) {
                if(lp.cur_iter % 97 == 0)
                    std::cout << '\r' << lp.progress() << "   " << bits::u128_tostring(lp.cur_iter);

                check(lp.state);
                lp.next();
            }
            
            if(fexit())
                goto end;

            std::cout << std::endl << "completed\n" << std::endl;

            lp.init(lp.popcnt - 1);
        } while(lp.popcnt > 0);

        end:;
        std::cout << "stopping search with n = " << lp.popcnt << " at iteration " << 
            bits::u128_tostring(lp.cur_iter) << " / " << bits::u128_tostring(lp.max_iter) << 
            "\nlast state: " << bits::u128_tostring16(lp.state)  << '\n' << std::endl;
    }

    template<typename Ran>
    void rsearch(Ran& ran, bool dense) {
        while(!fexit())
            check(ran.next() | (dense ? ran.next() : 0));        
        
        std::lock_guard<std::mutex> lock(glbl::cout_mutex);
        std::cout << "stopping search\n";
    }

    void check(uint64_t magic) {
        for(unsigned sqr = 0; sqr < 64; ++sqr)
            for(int bsp: {0, 1})
                update_best(sqr, bsp, magic); 
    }

    void update_best(int sqr, bool bsp, uint64_t magic) {
        unsigned best_shamt = mc.best_shamt(sqr, bsp, magic);

        if(__builtin_expect(best_shamt > best[sqr][bsp].get_shamt(), 0)) {
            best[sqr][bsp].update(magic, best_shamt);

            std::lock_guard<std::mutex> lock(glbl::cout_mutex);
            std::cout << "found new " << 
                (bsp ? 'b' : 'r') << char('a' + sqr % 8) << char('1' + sqr / 8) << 
                " key. shamt = " << 64 - best_shamt << "(" << mc.tbl.popcnt[sqr][bsp] << ")" <<
                '\n' << std::endl;
        }
    }
};

void go_ran(int thread_count) {
    using seqs::ran::xorshiro;

    magics::Magic** best = magics::init_best();
    const Tbls tbl;

    xorshiro* rans = new xorshiro[thread_count]; 
    Searcher** searchers = new Searcher*[thread_count]; 
    std::thread* threads = new std::thread[thread_count];

    for(int i = 0; i < thread_count; ++i) {
        searchers[i] = new Searcher(tbl, best);
        threads[i] = std::thread(&Searcher::rsearch<xorshiro>, searchers[i], std::ref(rans[i]), 1);
    }

    for(int i = 0; i < thread_count; ++i)
        threads[i].join();

    std::cout << "all threads joined succesfully\n\n";

    magics::write_best(best);
}

int main() {
    std::signal(SIGINT, handle_sigint);

    go_ran(5);

    std::cout << "exiting...\n";
    return 0;
}
