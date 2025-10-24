#include "types.h"
#include "bits.h"
#include "movegen.h"
#include "magic.h"

int main(int argc, char** argv) {

    struct magic rmagics[64], bmagics[64];
    magics_init(rmagics, bmagics);

    u32 s = H8;
    bitboard us = SQR_BB(C8);
    bitboard all = us | SQR_BB(H3);

    print_bb(magic_moves_bb(&rmagics[s], all, us));

    return 0;
}
