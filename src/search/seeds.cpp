#include "tbls.h"
#include "ran.h"
#include <atomic>
#include <initializer_list>
#include <iostream>
#include <vector>
#include <thread>
#include <pthread.h>

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

    start_search<ran::xorshift>(1);

    return 0;
}
