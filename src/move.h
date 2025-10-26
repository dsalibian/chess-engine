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

#define MV_GETCODE(m)               ((m) & 0xfu)
#define MV_GETTO(m)                 (((m) >> 4) & 0x3fu)
#define MV_GETFROM(m)               ((m) >> 10)

#define MV_ISCASTLE(m)              ((MV_GETCODE(m) == 0x2u) || (MV_GETCODE(m) == 0x3u))
#define MV_PROMOTYPE(m)             ((MV_GETCODE(m) & ~MCODE_FPROMO) + TYPE_NIGHT)
#define MV_ISENPASSANT(m)           (MV_GETCODE(m) == MCODE_EN_PASSANT)
#define MV_ISPROMO(m)               (m & MCODE_FPROMO)
#define MV_ISCAP(m)                 (m & MCODE_FCAP)

#define MV_ENCODE(code, to, from)   ((code) | ((to) << 4) | ((from) << 10))

typedef u16 move;

void dbgprint_mv(const move, const u32*, const bool);

#endif
