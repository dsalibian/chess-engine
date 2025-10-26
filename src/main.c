#include "misc.h"
#include "types.h"
#include "bits.h"
#include "movegen.h"
#include "magic.h"
#include "move.h"
#include "position.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char** argv) {

    struct position p;
    parse_fen(&p, "1k5r/2pR4/1pQ5/pP3Rrp/2P5/7P/1P4PK/4q3 w - a6 0 36");

    dbgprint_pos(&p);


    return 0;
}
