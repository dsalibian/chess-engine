#ifndef MOVE_H
#define MOVE_H

#include "types.h"

#define MCODE_FCAP                  (1u << 2)
#define MCODE_FPROMO                (1u << 3)

#define MCODE_QUIET                 0x0u
#define MCODE_DBL_PUSH              0x1u
#define MCODE_CASTLE_K              0x2u
#define MCODE_CASTLE_Q              0x3u
#define MCODE_CAPTURE               MCODE_FCAP
#define MCODE_EN_PASSANT            (MCODE_FCAP | 0x1u)

#define MCODE_PROMO_N               (MCODE_FPROMO | 0x0u)
#define MCODE_PROMO_B               (MCODE_FPROMO | 0x1u)
#define MCODE_PROMO_R               (MCODE_FPROMO | 0x2u)
#define MCODE_PROMO_Q               (MCODE_FPROMO | 0x3u)

#define MCODE_ISCASTLE(c)           ((c) == MCODE_CASTLE_K || (c) == MCODE_CASTLE_Q)
#define MCODE_ISPROMO(c)            ((c) & MCODE_FPROMO)
#define MCODE_ISCAP(c)              ((c) & MCODE_FCAP)
#define MCODE_PROMOTYPE(c)          (((c) & ~(MCODE_FPROMO | MCODE_FCAP)) + TYPE_NIGHT)

#define MV_GETCODE(m)               ((m) & 0xfu)
#define MV_GETTO(m)                 (((m) >> 4) & 0x3fu)
#define MV_GETFROM(m)               ((m) >> 10)

#define MV_ENCODE(code, to, from)   ((move)((code) | ((to) << 4) | ((from) << 10)))



typedef u16 move;

void uciprint_mv(const move, const bool);



struct move_stack {
    move ls[255];
    size_t size;
};

struct move_stack stack_new();
move stack_pop(struct move_stack*);
void stack_push(struct move_stack*, const move);
void stack_print(struct move_stack*);

#endif
