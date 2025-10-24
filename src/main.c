#include "types.h"
#include "bits.h"
#include "movegen.h"

int main(int argc, char** argv) {

    for(u32 s = 63; s < 64; --s)
        print_bb(gen_rmask(s, true));


    return 0;
}
