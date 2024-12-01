#include "position.h"
#include "misc.h"
#include "types.h"
#include <cctype>
#include <iostream>


Position::Position(): 
    Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

Position::Position(std::string fen) {
    for(int i = 0; i < 12; ++i) 
        bbs[i] = 0;

    parse_fen(fen);  
    set_combined();
}

void Position::parse_fen(std::string fen) {
    int i = 0;
    
    for(int rank = 7, file = 0; fen[i] != ' '; ++i) {
        char c = fen[i];

        if(c == '/') {
            --rank;   
            file = 0;
            continue;
        }

        if(std::isdigit(c)) {
            file += c - '0';
            continue;
        }
        
        Occupancy p;
        switch (c) {
            case 'P': p = w_pawn;   break;
            case 'N': p = w_night;  break;
            case 'B': p = w_bishop; break;
            case 'R': p = w_rook;   break;
            case 'Q': p = w_queen;  break;
            case 'K': p = w_king;   break;

            case 'p': p = b_pawn;   break;
            case 'n': p = b_night;  break;
            case 'b': p = b_bishop; break;
            case 'r': p = b_rook;   break;
            case 'q': p = b_queen;  break;
            case 'k': p = b_king;   break;
            
            default: p = empty;
        }

        if(p != empty)
            bbs[p] |= 1ull << (8 * rank + file);
        

        file = (file + 1) & 0xF; 
    }

    turn_w = fen[++i] == 'w';
    i += 2;

    oo_w = oo_b = ooo_w = ooo_b = false;
    for(; fen[i] != ' '; ++i) {
        char c = fen[i];
        switch(c) {
            case 'K': oo_w  = true; break;
            case 'Q': ooo_w = true; break;
            case 'k': oo_b  = true; break;
            case 'q': ooo_b = true; break;
        }
    }
    ++i;

    if(fen[i] == '-') 
        en_passant = 0;
    else {
        en_passant = (fen[i] - 'a') * 8 + fen[i + 1] - '0';
        ++i;
    }
    i += 2;
    
    std::string num("");
    for(; fen[i] != ' '; ++i)
        num += fen[i];
    hmoves = std::stoi(num);
    ++i;

    num = "";
    for(; fen[i] != '\0'; ++i)
        num += fen[i];
    fmoves = std::stoi(num);
}

void Position::set_combined() {
    w_bb = b_bb = 0;

    for(int i = w_pawn; i <= b_king; ++i)
        (i <= w_king ? w_bb : b_bb) |= bbs[i];

    all_bb = w_bb | b_bb;
    us = &w_bb;
    them = &b_bb;
}

Occupancy Position::piece_at(int sqr) {
    for(int i = 0; i < 12; ++i)
        if((bbs[i] >> sqr) & 1)
            return Occupancy(i);

    return Occupancy::empty;
}

const char* piece_lchars[13] {
    u8"♟", u8"♞", u8"♝", u8"♜", u8"♛", u8"♚",
    u8"♙", u8"♘", u8"♗", u8"♖", u8"♕", u8"♔",
    u8"."
};

const char piece_chars[13] {
    'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k',
    '.'
};

void Position::print(bool unicode) {
    for(int r = 7; r >= 0; --r) {
        std::cout << ' ' << r + 1 << "   ";
        for(int f = 0; f <= 7; ++f) {
            if(unicode)
                std::cout << piece_lchars[piece_at(8 * r + f)] << ' ';
            else
                std::cout << piece_chars[piece_at(8 * r + f)] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\n     a b c d e f g h\n\n";
}

void Position::print_info() {
    std::cout << "turn:  " << (turn_w ? 'w' : 'b') << '\n'; 

    std::cout << "oo_w:  " << (oo_w  ? 'y' : 'n') << '\n'; 
    std::cout << "ooo_w: " << (ooo_w ? 'y' : 'n') << '\n'; 
    std::cout << "oo_b:  " << (oo_b  ? 'y' : 'n') << '\n'; 
    std::cout << "ooo_b: " << (ooo_b ? 'y' : 'n') << '\n'; 

    std::cout << "en passant: " << 
        (en_passant ? sqr_tostring(en_passant) : "n") << '\n';

    std::cout << "half-moves: " << hmoves << '\n';
    std::cout << "full-moves: " << fmoves << "\n\n" << std::endl;
}

void Position::print_full(bool unicode) {
    print(unicode);
    print_info();
}
