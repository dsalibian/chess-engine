#ifndef MOVEMAKE_H
#define MOVEMAKE_H

#include "types.h"
#include "position.h"
#include "move.h"
#include "movegen.h"

void move_make(const struct atts_tbl*, struct position*, const move);

#endif
