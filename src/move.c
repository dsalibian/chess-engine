#include "move.h"
#include "bits.h"
#include "misc.h"
#include <stdio.h>

char mv_promochar(const move m) {
    static const char arr[] = {'n', 'b', 'r', 'q'};

    return arr[MCODE_PROMOTYPE(MV_GETCODE(m)) - 1];
}

extern char type_char(const u8);

void uciprint_mv(const move m, const bool println) {
    const u32 code = MV_GETCODE(m); 
    const u32 from = MV_GETFROM(m); 
    const u32 to   = MV_GETTO(m); 

    printf("%c%c%c%c", 
            'a' + SQR_FILE(from),
            '1' + SQR_RANK(from),
            'a' + SQR_FILE(to),
            '1' + SQR_RANK(to));

    if(MCODE_ISPROMO(code))
        printf("%c", mv_promochar(m));

    if(println)
        printf("\n");
}

struct move_stack stack_new() {
    return (struct move_stack){.size = 0};
}

move stack_pop(struct move_stack* ms) {
    assert(ms->size);

    return ms->ls[--(ms->size)];
}

void stack_push(struct move_stack* ms, const move m) {
    assert(ms->size < 255);

    ms->ls[(ms->size)++] = m;
}

void stack_print(struct move_stack* ms) {
    const size_t u = ms->size;

    for(size_t i = 0; i < u; ++i) {
        printf("%3lu: ", i);
        uciprint_mv(ms->ls[i], true);
    }
}
