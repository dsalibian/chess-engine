#include "types.h"
#include "bits.h"
#include "movegen.h"

int main(int argc, char** argv) {

    for(u32 s = 0; s < 64; ++s)
        print_bb(gen_ratts(s, SQR_BB(H5) | SQR_BB(A5)));


    return 0;
}
