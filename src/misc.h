#ifndef MISC_H
#define MISC_H

#include <iostream>

using namespace std;
typedef unsigned long long ull;

void print_bitboard(ull);
bool get_bit(ull, int);
void set_bit(ull&, int, bool);
int pop_lsb(ull&);

#endif