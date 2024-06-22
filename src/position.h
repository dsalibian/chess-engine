#ifndef POSITION_H
#define POSITION_H

#include <array>
#include <string>

#include "movegen.h"

using namespace std;
typedef unsigned long long ull;
typedef unsigned short ushort;

struct Position {
    array<ull, 12> bitboards;
    ull w_pieces;
    ull b_pieces;
    ull all_pieces;

    Position();

};

/*
 *  bits 0-5 : to
 *       6-11: from
 *       12+ : move code
 */
struct Move {
    ushort dat;

    Move(int to, int from, int code) {
        dat = ( code << 12 ) | ( from << 6 ) | to;
    }

    int get_to() { return dat & 0x3F; }
    ull get_toll() { return BB(get_to()); }

    int get_from() { return ( dat >> 6 ) & 0x3F; }
    ull get_fromll() { return BB(get_from()); }

    int get_code() { return dat >> 12; }
    ull get_codell() { return BB(get_code()); }

};

#endif
