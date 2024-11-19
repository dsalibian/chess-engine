#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <chrono>
#include <initializer_list>
#include <atomic>
#include <mutex>
#include <csignal>
#include <thread>
#include <filesystem>
#include <fstream>

using std::uint64_t;
using bitboard = std::uint64_t;

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

} // namespace bits


namespace ran {

struct PRNG {
    virtual uint64_t next() = 0;  
    virtual uint64_t sparse() final {
        return next() & next() & next();
    }
};

// https://prng.di.unimi.it/splitmix64.c
struct splitmix: PRNG {
    static constexpr int size = 1;
    uint64_t state[size];

    splitmix() { 
        *state = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
        for(int i = 0; i < 0x999; ++i) 
            next(*state);
    }

    splitmix(uint64_t seed) { 
        *state = seed; 
    }

    uint64_t next() override {
        uint64_t t = (*state += 0x9e3779b97f4a7c15ULL);
        t = (t ^ (t >> 30)) * 0xbf58476d1ce4e5b9ULL;
        t = (t ^ (t >> 27)) * 0x94d049bb133111ebULL;
        return t ^ (t >> 31);
    }

    static void next(uint64_t& state) {
        state += 0x9e3779b97f4a7c15ULL;
        state = (state ^ (state >> 30)) * 0xbf58476d1ce4e5b9ULL;
        state = (state ^ (state >> 27)) * 0x94d049bb133111ebULL;
        state ^= (state >> 31);
    }
};

void seed(uint64_t* state, int size) {
    splitmix m;
    for(int i = 0; i < size; ++i) 
        state[i] = m.next();
}

// https://en.wikipedia.org/wiki/Xorshift#Example_implementation
struct xorshift: PRNG {
    static constexpr int size = 1;
    uint64_t state[size];

    xorshift() { 
        seed(state, size); 
    }

    uint64_t next() override {
        *state ^= *state << 7; 
        *state ^= *state >> 9; 
        return *state;
    }
};

// https://prng.di.unimi.it/xoshiro256plusplus.c
struct xorshiro: PRNG {
    static constexpr int size = 4;
    uint64_t state[size];    

    xorshiro() { 
        seed(state, size); 
    };

    uint64_t rotl(uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    uint64_t next() override {
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
struct mwc: PRNG {
    static constexpr int size = 3;
    static constexpr __uint128_t MWC_A2 = 0xffa04e67b3c95d86ULL;
    uint64_t state[size];

    mwc() { 
        seed(state, size); 
        state[2] ? (state[2] &= MWC_A2 - 2) : state[2] = 1;
    }

    uint64_t next() override {
        const uint64_t result = state[1];
        const __uint128_t t = MWC_A2 * state[0] + state[2];
        state[0] = state[1];
        state[1] = uint64_t(t);
        state[2] = uint64_t(t >> 64);
        return result;
    }
};

// https://github.com/imneme/pcg-cpp/blob/master/include/pcg_random.hpp
// https://github.com/lemire/testingRNG/blob/master/source/pcg64.h#L20
struct pcg: PRNG {
    static constexpr int size = 2; 
    static constexpr __uint128_t m = (__uint128_t(2549297995355413924ULL) << 64) | 4865540595714422341ULL;
    static constexpr __uint128_t a = (__uint128_t(6364136223846793005ULL) << 64) | 1442695040888963407ULL;
    uint64_t state[2];

    pcg() { 
        seed(state, size); 
    }

    uint64_t rotr64(uint64_t x, unsigned k) {
        return (x >> k) | (x << ((-k) & 63));
    }

    uint64_t rotr128(__uint128_t x) {
        return rotr64(uint64_t(x >> 64) ^ uint64_t(x), unsigned(x >> 122));
    }

    uint64_t next() override {
        __uint128_t s = (__uint128_t(state[1]) << 64) | state[0];
        __uint128_t t = s * m + a;
        state[0] = uint64_t(t);
        state[1] = uint64_t(t >> 64);
        return rotr128(t);
    }
};

} // namespace ran

namespace glbl {
    std::atomic<bool> fexit{false};
    std::mutex cout_mutex; 
}

bool read_fexit() {
    return glbl::fexit.load(std::memory_order_relaxed);
}

void handle_sigint(int signal) {
    std::cout << "\nsigint signal " << signal << " received...\n\n";
    glbl::fexit.store(true, std::memory_order_relaxed);
    // std::exit(0);
}

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

        for(int sqr = 0; sqr < 64; ++sqr) {
            rmask[sqr]  = new bitboard[2];
            popcnt[sqr] = new int[2];
            u[sqr]      = new int[2];
            shamt[sqr]  = new int[2];
            perms[sqr]  = new bitboard*[2];
            moves[sqr]  = new bitboard*[2];

            for(int bsp: {0, 1}) {
                rmask [sqr][bsp] = movegen::rmask(sqr, bsp);
                popcnt[sqr][bsp] = __builtin_popcountll(rmask[sqr][bsp]);
                u     [sqr][bsp] = 1U << popcnt[sqr][bsp];
                shamt [sqr][bsp] = 64  - popcnt[sqr][bsp];
                perms [sqr][bsp] = new bitboard[u[sqr][bsp]];
                moves [sqr][bsp] = new bitboard[u[sqr][bsp]];
            }

            for(int bsp: {0, 1}) {
                for(int i = 0; i < u[sqr][bsp]; ++i) {
                    perms[sqr][bsp][i] = bits::occ_mask(i, popcnt[sqr][bsp], rmask[sqr][bsp]);
                    moves[sqr][bsp][i] = movegen::satts(sqr, perms[sqr][bsp][i], bsp);
                }
            }
        }
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

struct MChecker {
    Tbls& tbl;
    bitboard* atts;
    uint64_t* iters, iter;

    MChecker(Tbls& _tbl): tbl(_tbl) {
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

        for(int j = 0; j < tbl.u[sqr][bsp]; ++j) {
            unsigned k = unsigned((magic * tbl.perms[sqr][bsp][j]) >> (shamt ? *shamt : tbl.shamt[sqr][bsp]));

            if(iters[k] != iter) {
                iters[k] = iter;
                atts[k] = tbl.moves[sqr][bsp][j];
            }
            else if(atts[k] != tbl.moves[sqr][bsp][j])
                return false;
            
        }
        return true;
    }

    unsigned best_shamt(int sqr, bool bsp, uint64_t magic) {
        for(int i = 61; i >= tbl.shamt[sqr][bsp]; --i)
            if(valid_magic(sqr, bsp, magic, i))
                return i;

        return 0; 
    }
};

struct LPerm {
    unsigned popcnt;
    uint64_t state, max_iters, cur_iter;
    static constexpr int max_k = 26, min_k = 39;
    
    LPerm(unsigned _popcnt) {
        popcnt = _popcnt;
        max_iters = nC(64, popcnt);
        cur_iter = 0;

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

    // good for n = 64, k < 27 or k > 38
    static constexpr uint64_t nC(unsigned n, unsigned k) {
        if( !k ) return 1;
        return nC(n - 1, k - 1) * n / k;
    }
};


namespace Searcher {

template<typename prng>
struct Best_Seed {
    MChecker mc;
    prng ran;
    std::atomic<unsigned>& glbl_min;
    uint64_t* pstate;
    
    Best_Seed(Tbls& _tbl, std::atomic<unsigned>& _glbl_min):
        mc(_tbl), glbl_min(_glbl_min) {
            pstate = new uint64_t[prng::size];
        }

    ~Best_Seed() {
        delete[] pstate;
    }

    void search() {
        while(!read_fexit()) {
            jmp();
            unsigned count = count_its();
            if(count) {
                print_pstate(count);
                glbl_min.store(count, std::memory_order_relaxed);
            }
        }
    }

    void jmp() {
        do write_pstate();
        while (!mc.valid_magic(0, 0, ran.sparse()));
        read_pstate();
    }

    unsigned read_gmin() {
        return glbl_min.load(std::memory_order_relaxed);
    }

    unsigned count_its() {
        unsigned count = 0;

        for(int sqr = 0; sqr < 64; ++sqr)
            for(bool bsp: {0, 1})
                if(!next_magic(sqr, bsp, count)) 
                    return 0;

        return count;
    }

    bool next_magic(int sqr, bool bsp, unsigned& count) {
        uint64_t magic;

        for(; count < read_gmin(); ++count) {
            do magic = ran.sparse();
            while (!magic);

            if(mc.valid_magic(sqr, bsp, magic)) 
                return true;
        }

        return false;
    }

    void print_pstate(unsigned count) {
        std::lock_guard<std::mutex> lock(glbl::cout_mutex);

        std::cout << "seeds: ";
        for(int i = 0; i < prng::size; ++i)
            std::cout << "s[" << std::dec << i << "] = 0x" << std::hex << pstate[i] << "   ";
        std::cout << "\niters = " << std::dec << count << '\n' << std::endl;
    }

    void write_pstate() {
        for(int i = 0; i < prng::size; ++i)
            pstate[i] = ran.state[i];
    }

    void read_pstate() {
        for(int i = 0; i < prng::size; ++i)
            ran.state[i] = pstate[i];
    }

};

struct Best_Magic {
    MChecker mc;
    std::atomic<uint64_t>** magics;
    std::atomic<unsigned>** shamts;

    Best_Magic(Tbls& _tbl, std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**> _bests): 
        mc(_tbl) { 
            magics = _bests.first;
            shamts = _bests.second;
    }

    void search(LPerm& seq) {
        while(seq.cur_iter < seq.max_iters && !read_fexit()) {
            for(int sqr = 0; sqr < 64; ++sqr)
                for(int bsp: {0, 1})
                    update_best(sqr, bsp, seq.state);

            seq.next();
        } 
    }

    template<typename prng>
    void rsearch(prng& ran, bool dense) {
        while(!read_fexit()) {
            uint64_t m = ran.next() | (dense ? ran.next() : 0);

            for(unsigned sqr = 0; sqr < 64; ++sqr)
                for(int bsp: {0, 1})
                    update_best(sqr, bsp, m); 
        }
    } 

    void write_magic(int sqr, bool bsp, uint64_t magic) {
        magics[sqr][bsp].store(magic, std::memory_order_relaxed);
    }

    uint64_t read_shamt(int sqr, bool bsp) {
        return shamts[sqr][bsp].load(std::memory_order_relaxed);
    }

    void update_best(int sqr, bool bsp, uint64_t magic) {
        unsigned bshamt = mc.best_shamt(sqr, bsp, magic);

        if(bshamt > read_shamt(sqr, bsp)) {
            write_magic(sqr, bsp, magic);
            shamts[sqr][bsp].store(bshamt, std::memory_order_relaxed);

            std::lock_guard<std::mutex> lock(glbl::cout_mutex);
        }
    }

    static std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**> generate_bests() {
        std::atomic<uint64_t>** magics = new std::atomic<uint64_t>*[64];
        std::atomic<unsigned>** shamts = new std::atomic<unsigned>*[64];

        Tbls tbl;
        MChecker mc(tbl);
        ran::pcg ran;
        uint64_t magic;

        for(int sqr = 0; sqr < 64; ++sqr) {
            magics[sqr] = new std::atomic<uint64_t>[2];
            shamts[sqr] = new std::atomic<unsigned>[2];
            for(int bsp: {0, 1}) {
                do {
                    do magic = ran.sparse();
                    while(!magic);
                } while(!mc.valid_magic(sqr, bsp, magic));

                magics[sqr][bsp] = magic;
                shamts[sqr][bsp] = mc.best_shamt(sqr, bsp, magic);
            }
        } 

        return std::make_pair(magics, shamts);
    }

    static std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**>  read_bests() {
        std::atomic<uint64_t>** magics = new std::atomic<uint64_t>*[64];
        std::atomic<unsigned>** shamts = new std::atomic<unsigned>*[64];

        std::ifstream in("dat", std::ios::binary);

        uint64_t magic_rk, magic_bsp;
        unsigned shamt_rk, shamt_bsp;

        for(int sqr = 0; sqr < 64; ++sqr) {
            in.read(reinterpret_cast<char*>(&magic_rk),  sizeof(magic_rk));
            in.read(reinterpret_cast<char*>(&shamt_rk),  sizeof(shamt_rk));
            in.read(reinterpret_cast<char*>(&magic_bsp), sizeof(magic_bsp));
            in.read(reinterpret_cast<char*>(&shamt_bsp), sizeof(shamt_bsp));

            magics[sqr][0].store(magic_rk,  std::memory_order_relaxed);
            magics[sqr][0].store(shamt_rk,  std::memory_order_relaxed);
            magics[sqr][1].store(magic_bsp, std::memory_order_relaxed);
            magics[sqr][1].store(shamt_bsp, std::memory_order_relaxed);
        }

        in.close();
        return std::make_pair(magics, shamts);
    }

    static void write_bests(std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**> bests) {
        std::cout << "writing best_magics...\n";

        if(!std::filesystem::exists("dat"))
            system("touch dat");

        std::ofstream out("dat", std::ios::binary);

        for(int sqr = 0; sqr < 64; ++sqr) {
            uint64_t magic_rk  = bests.first [sqr][0].load(std::memory_order_relaxed);         
            unsigned shamt_rk  = bests.first [sqr][0].load(std::memory_order_relaxed);         
            uint64_t magic_bsp = bests.second[sqr][1].load(std::memory_order_relaxed);         
            unsigned shamt_bsp = bests.second[sqr][1].load(std::memory_order_relaxed);         

            out.write(reinterpret_cast<const char*>(&magic_rk),  sizeof(magic_rk));
            out.write(reinterpret_cast<const char*>(&shamt_rk),  sizeof(shamt_rk));
            out.write(reinterpret_cast<const char*>(&magic_bsp), sizeof(magic_bsp));
            out.write(reinterpret_cast<const char*>(&shamt_bsp), sizeof(shamt_bsp));
        }

        out.close();
    }

    static void write_bests_pretty(std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**> bests) {
        std::cout << "writing best_magics_pretty...\n";
        Tbls tbl;

        if(!std::filesystem::exists("best_magics"))
            system("touch best_magics");

        std::ofstream out("best_magics");

        for(int sqr = 0; sqr < 64; ++sqr) {
            uint64_t magic_rk  = bests.first [sqr][0].load(std::memory_order_relaxed);         
            unsigned shamt_rk  = bests.first [sqr][0].load(std::memory_order_relaxed);         
            uint64_t magic_bsp = bests.second[sqr][1].load(std::memory_order_relaxed);         
            unsigned shamt_bsp = bests.second[sqr][1].load(std::memory_order_relaxed);         

            

        }

        out.close();
    }

    static std::pair<std::atomic<uint64_t>**, std::atomic<unsigned>**> init_bests() {
        if(std::filesystem::exists("dat"))
            return read_bests();

        return generate_bests();
    }
};

} // namespace searcher


template<typename Ran> 
void seed_search(int thread_count) {
    using Searcher::Best_Seed, std::thread;

    Tbls tbl;
    std::atomic<unsigned> min(~0U);

    thread* threads = new thread[thread_count];
    Best_Seed<Ran>** searchers = new Best_Seed<Ran>*[thread_count];

    for(int i = 0; i < thread_count; ++i) {
        searchers[i] = new Best_Seed<Ran>(tbl, min);
        threads[i] = thread(&Best_Seed<Ran>::rsearch, searchers[i]);
    }
    
    for(int i = 0; i < thread_count; ++i)
        threads[i].join();

    std::cout << "all threads joined succesfully...\n";
}

void magic_search_exh(bool multithread, bool dense) {
    using Searcher::Best_Magic, std::thread;

    auto bests = Best_Magic::init_bests();
    Tbls tbl;

    if(multithread) {
        Best_Magic *searcher0 = new Best_Magic(tbl, bests), 
                   *searcher1 = new Best_Magic(tbl, bests); 

        for(int i = dense ? 63 : 1; !read_fexit() ; i += dense ? -2 : 2) {
            LPerm seq0(i), seq1(i + (dense ? -1 : 1));

            std::cout << "i = " << i << ", " << i + (dense ? -1 : 1) << std::endl;
            thread t0(&Best_Magic::search, searcher0, std::ref(seq0));
            thread t1(&Best_Magic::search, searcher0, std::ref(seq1));

            t0.join();
            t1.join();
        }

    } else {
        Best_Magic searcher(tbl, bests); 
         
        for(int i = dense ? 63 : 1; !read_fexit() ; i += dense ? -1 : 1) {
            LPerm seq(i);
            std::cout << "i = " << i << std::endl;
            searcher.search(seq);
        }
    }

    Best_Magic::write_bests(bests);
}

template<typename Ran> 
void magic_search_ran(int thread_count, int dense_count) {
    using Searcher::Best_Magic, std::thread;

    auto bests = Searcher::Best_Magic::init_bests();
    Tbls tbl;
    Ran ran;

    thread* threads = new thread[thread_count];
    Best_Magic** searchers = new Best_Magic*[thread_count];
    
    for(int i = 0; i < thread_count; ++i) {
        searchers[i] = new Best_Magic(tbl, bests);
        threads[i] = thread(&Best_Magic::rsearch<Ran>, searchers[i], std::ref(ran), i < dense_count);
    }

    for(int i = 0; i < thread_count; ++i)
        threads[i].join();


    Best_Magic::write_bests(bests);
}


int main(int argc, char **argv) {
    std::signal(SIGINT, handle_sigint);

    magic_search_ran<ran::xorshiro>(1, 0);

    std::cout << "exiting...\n";
    return 0;
}
