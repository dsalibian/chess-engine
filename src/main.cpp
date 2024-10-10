#include "types.h"
#include "misc.h"
#include "movegen.h"
#include "position.h"
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

extern Magic rmagics[64];
extern Magic bmagics[64];
extern bitboard patts_tbl[64][2];

extern int t;
int main() {
    using namespace std;

    movegen::init_atts();
    cout << dec << t << '\n';


    // bitboard t = movegen::rmask(e4, 0);

    for(int i = 0; i < 100; ++i) {
        print_bb(occ_mask(i, 12, movegen::rmask(0, 0)));
    }
        

    return 0;
}
