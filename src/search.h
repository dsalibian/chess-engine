#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "movegen.h"

i32 qss(const struct atts_tbl*, const struct position*, i32, i32);

i32 nega(const struct atts_tbl*, const struct position*, u32, i32, i32, bool);

#endif
