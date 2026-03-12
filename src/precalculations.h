#pragma once
#include <cstdint>
#include <array>

extern const std::array<uint64_t, 64> KING_ATTACKS;
extern const std::array<uint64_t, 64> KNIGHT_ATTACKS;
extern const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS;
extern const std::array<uint64_t, 64> BISHOP_RAYS;
extern const std::array<uint64_t, 64> ROOK_RAYS;
extern const std::array<uint64_t, 64> QUEEN_RAYS;

extern const std::array<uint64_t, 64> BISHOP_MASK;
extern const std::array<uint64_t, 64> ROOK_MASK;


extern const std::array<std::array<uint64_t,512>, 64> BISHOP_ATTACKS; 
extern const std::array<std::array<uint64_t, 4096>, 64> ROOK_ATTACKS; 

extern const std::array<std::array<uint64_t,64>,2> PASSED_PAWNS;

// Stores rays for each position and each direction
// BOARD_RAYS[0] stores the rays for NORTH, continues in clockwise direction
extern const std::array<std::array<uint64_t,64>, 8> BOARD_RAYS;
/*
    Example BOARD_RAYS[0][10]

    8   0 0 1 0 0 0 0 0
    7   0 0 1 0 0 0 0 0
    6   0 0 1 0 0 0 0 0
    5   0 0 1 0 0 0 0 0
    4   0 0 1 0 0 0 0 0
    3   0 0 1 0 0 0 0 0
    2   0 0 0 0 0 0 0 0
    1   0 0 0 0 0 0 0 0

        A B C D E F G H


    Example BOARD_RAYS[1][11]

    8   0 0 0 0 0 0 0 0
    7   0 0 0 0 0 0 0 0
    6   0 0 0 0 0 0 0 1
    5   0 0 0 0 0 0 1 0
    4   0 0 0 0 0 1 0 0
    3   0 0 0 0 1 0 0 0
    2   0 0 0 0 0 0 0 0
    1   0 0 0 0 0 0 0 0

        A B C D E F G H
*/

extern const std::array<uint64_t,8> BOARD_COLUMNS;

extern const std::array<std::array<std::array<uint64_t, 64>, 7>,2>  PIECE_ZOBRIST;
extern const uint64_t COLOR_ZOBRIST;
extern const uint64_t BOARD_ZOBRIST;
extern const std::array<uint64_t, 16> CASTLING_ZOBRIST;
extern const std::array<uint64_t, 8> ENPASSANT_ZOBRIST;

extern const std::array<std::array<int,256>, 64> LMR_VALUES;
