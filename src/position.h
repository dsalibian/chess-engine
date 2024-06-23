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
    int code;
    ull to;
    ull from;
    Move(int code, ull to, ull from) {
        this -> code = code;
        this -> to = to;
        this -> from = from;
    }
};

#endif
