#include "types.h"
#include "misc.h"
#include "movegen.h"
#include "position.h"
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

extern int magic_search_iters;

int main() {
    using namespace std;

    MoveGen tbl;

    Position p("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
    p.print_full(1);

    return 0;
}
