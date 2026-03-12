#pragma once
#include <cstdint>
#include <climits>

constexpr int MAX = INT16_MAX;
constexpr int MIN = INT16_MIN;
constexpr int MATE = 31000;

// 4194304 = 2^22
constexpr int TT_SIZE = 4194304;

constexpr int pieceValue[2][7] = {{0, 100, 310, 320, 500, 900, 0},{0, -100, -310, -320, -500, -900, 0}};

constexpr int piecePhase[7] = {0, 0, 1, 1, 3, 6, 0};

constexpr int MVV_LVA[7][7] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, 105,  205,  305,  405,  505,  605 },
    {0, 104,  204,  304,  404,  504,  604 },
    {0, 103,  203,  303,  403,  503,  603 },
    {0, 102,  202,  302,  402,  502,  602 },
    {0, 101,  201,  301,  401,  501,  601 },
    {0, 100,  200,  300,  400,  500,  600 }
};

enum ScoreType {
    EXACT, LOWER, UPPER
};

enum KnightOffsets {
    // first offset is in upper right corner, continues in clockwise direction
    KNIGHT_OFFSET_1 = 17,
    KNIGHT_OFFSET_2 = 10,
    KNIGHT_OFFSET_3 = -6,
    KNIGHT_OFFSET_4 = -15,
    KNIGHT_OFFSET_5 = -17,
    KNIGHT_OFFSET_6 = -10,
    KNIGHT_OFFSET_7 = 6,
    KNIGHT_OFFSET_8 = 15
};

enum Direction {
    NORTH = 8,
    SOUTH = -8,
    WEST = -1,
    EAST = 1,
    NORTH_WEST = 7,
    NORTH_EAST = 9,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

enum MoveType {
    NORMAL,
    PROMOTION,
    CASTLE,
    EN_PASSANT
};

enum PromotionType {
    P_KNIGHT,
    P_BISHOP,
    P_ROOK,
    P_QUEEN
};

enum Square { A1,B1,C1,D1,E1,F1,G1,H1,
              A2,B2,C2,D2,E2,F2,G2,H2,
              A3,B3,C3,D3,E3,F3,G3,H3,
              A4,B4,C4,D4,E4,F4,G4,H4,
              A5,B5,C5,D5,E5,F5,G5,H5,
              A6,B6,C6,D6,E6,F6,G6,H6,
              A7,B7,C7,D7,E7,F7,G7,H7,
              A8,B8,C8,D8,E8,F8,G8,H8, NONE
};

enum Piece {
    EMPTY, W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN=9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
};

enum PieceType {
    EMPTY_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

enum Color {
    WHITE,
    BLACK,
};

