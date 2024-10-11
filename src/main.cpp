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


        

    return 0;
}
