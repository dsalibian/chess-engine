#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "position.h"
#include "movegen.h"

#define EVAL_MATE 999999
#define EVAL_DRAW 0

i32 eval(const struct atts_tbl*, const struct position*);

#endif
