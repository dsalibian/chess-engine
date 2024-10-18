#include "tbls.h"
#include "ran.h"
#include <iostream>

// good for n = 63, k <= 26
uint64 nC(unsigned n, unsigned k) {
    if( !k ) return 1;
    return nC(n - 1, k - 1) * n / k;
}

template<typename fun>
struct Magic_Checker {
    Tbls* tbl;
    bitboard* atts;
    unsigned* iters, total_its = 0;
    fun next();

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

    uint64 next_magic(int pos, bool bsp) {
        uint64 magic;
        for(;;) {
            do magic = next();
            while (!magic);

            if( valid_magic(pos, bsp, magic)) 
                return magic;
        }
    }

};

template<typename Ran>
struct Ran_Searcher {

};

struct Exhaustive_Searcher {

};

int main() {
    using namespace std;


    return 0;
}
