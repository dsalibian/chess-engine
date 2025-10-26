#ifndef MOVEMAKE_H
#define MOVEMAKE_H

#include "types.h"
#include "position.h"
#include "move.h"

struct umake_dat {
    u32 en_passant_target;
    u32 captured_type;
    u32 hmoves;
    bool castle_k[2], castle_q[2];
};

void move_make(struct position*, const move);
void move_unmake(struct position*, const move, const struct umake_dat*);

struct umake_dat set_umakedat(const struct position*, const move);

#endif
