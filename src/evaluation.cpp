#include "evaluation.h"
#include "board.h"
#include "constants.h"
#include "helperFunctions.h"
#include "precalculations.h"
#include <immintrin.h>


// the piece square tables are from https://www.chessprogramming.org/Simplified_Evaluation_Function
int pieceSquareTable[7][64] = {
    {0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    },
//pawn
{

    0,  0,  0,  0,  0,  0,  0,  0,
    30, 30, 30, 35, 35, 30, 30, 30,
    10, 10, 15, 20, 20, 15, 10, 10,
    8,  8,  10, 20, 20, 10, 8,  8,
    3,  3,  10, 20, 20, 10, 3,  3,
    5,  -5,  -5,  5,  5,  -5,  -5,  5,
    3,  5,  5,  -10, -10, 5,  5,  3,
    0,  0,  0,  0,  0,  0,  0,  0,
},
    //knight
    {
        -20, -10, -10,  -10,  -10,  -10,  -10, -20,
        -10, -5,  0,  0,  0,  0,  -5,  -10,
        -10, 0,  10, 10, 10, 10, 0,  -10,
        -10, 0,  10, 15, 15, 10, 0,  -10,
        -10, 0,  10, 15, 15, 10, 0,  -10,
        -10, 0,  15, 10,  10,  15, 0,  -10,
        -10, -5,  0,  0,  0,  0,  -5,  -10,
        -20, -10, -10,  -10,  -10,  -10,  -10, -20,
    },

    //bishop
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },

//rook
{
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  3,  0,  0

},

//queen
{
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  0,  5,  5,  0,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
    -5,   0,  5,  5,  5,  5,  0, -5,
    -10,  0,  0,  5,  5,  0,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10, 5, -5,-10,-10,-20

},

//king

{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 0, 0, 5, 0, 0,
    0, 0, -5, -5, -5, -5, -5, 0,
    0, 0, 5, -5, -5, -5, 10, 0,
},
};

int endgamePieceSquareTable[7][64] = {
    {0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    },
    //pawn
    {

        0,  0,  0,  0,  0,  0,  0,  0,
        30, 30, 30, 35, 35, 30, 30, 30,
        10, 10, 15, 20, 20, 15, 10, 10,
        8,  8,  10, 20, 20, 10, 8,  8,
        6,  6,  10, 20, 20, 10, 6,  6,
        5,  -5,  -5,  5,  5,  -5,  -5,  5,
        3,  5,  5,  -10, -10, 5,  5,  3,
        0,  0,  0,  0,  0,  0,  0,  0,
    },
    //knight
    {
        -20, -10, -10,  -10,  -10,  -10,  -10, -20,
        -10, -5,  0,  0,  0,  0,  -5,  -10,
        -10, 0,  10, 10, 10, 10, 0,  -10,
        -10, 0,  10, 15, 15, 10, 0,  -10,
        -10, 0,  10, 15, 15, 10, 0,  -10,
        -10, 0,  15, 10,  10,  15, 0,  -10,
        -10, -5,  0,  0,  0,  0,  -5,  -10,
        -20, -10, -10,  -10,  -10,  -10,  -10, -20,
    },
    //bishop
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },

    //rook
    {
        -10,  0,  0,  0,  0,  0,  0,  -10,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        -10,  0,  0,  0,  0,  0,  0,  -10,

    },

    //queen
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  0,  5,  5,  0,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
        -5,   0,  5,  5,  5,  5,  0, -5,
        -10,  0,  0,  5,  5,  0,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, 5, -5,-10,-10,-20

    },

    //king

    {
        -35,  -25,  -20,  -20,  -20,  -20,  -25,  -35,
        -25, -10, -5, -5, -5, -5, -10,  -25,
       -20,  -5,  3,  3,  3,  3,  -5, -20,
       -20,  -5,  3,  10,  10,  3,  -5, -20,
       -20,  -5,  3,  10,  10,  3, -5, -20,
       -20,  -5,  3,  3,  3,  3,  -5, -20,
       -25,  -10,  -5,  -5,  -5,  -5,  -10, -25,
        -35,  -25,  -20,  -20,  -20,  -20,  -25,  -35
    },
};

int mirrorTable[] = {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
};

constexpr int knightMobility[9]  = { -50, -25, -5,  5, 15, 20, 27, 31, 35 };
constexpr int bishopMobility[14] = { -50, -20,  -15, -5, 5, 15, 25, 30, 35, 47, 49, 50, 50, 50};
constexpr int rookMobility[15]   = { -50, -20, -5,  0,  5,  10, 15, 20, 23, 24, 25, 26, 27, 27, 27 };
constexpr int queenMobility[28]  = { -30, -10, -5, 0,  0,  4,  8,  12, 15, 16, 17, 17, 17, 17,
                                      17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17};

int mobilityScore(Board& board, Color color) {
    Color otherColor = color == WHITE ? BLACK : WHITE;

    uint64_t pawnAttacks;
    if (otherColor == WHITE)
        pawnAttacks =  ((board.getPieceBitboard(WHITE, PAWN) << 9) & ~BOARD_COLUMNS[0])
                     | ((board.getPieceBitboard(WHITE, PAWN) << 7) & ~BOARD_COLUMNS[7]);
    else
        pawnAttacks = ((board.getPieceBitboard(BLACK, PAWN) >> 7) & ~BOARD_COLUMNS[0])
                         | ((board.getPieceBitboard(BLACK, PAWN) >> 9) & ~BOARD_COLUMNS[7]);

    uint64_t safeSquares = ~(board.getColorBitboard(color) | pawnAttacks);
    uint64_t occupied = board.getOccupiedBitboard();
    int score=0;

    uint64_t pieces = board.getPieceBitboard(color, KNIGHT);
    while (pieces) {
        int pos = popBit(pieces);
        score += knightMobility[countSetBits(KNIGHT_ATTACKS[pos] & safeSquares)];
    }

    pieces = board.getPieceBitboard(color, BISHOP);
    while (pieces) {
        int pos = popBit(pieces);
        int index = _pext_u64(occupied, BISHOP_MASK[pos]);
        score += bishopMobility[countSetBits(BISHOP_ATTACKS[pos][index] & safeSquares)];
    }

    pieces = board.getPieceBitboard(color, ROOK);
    while (pieces) {
        int pos = popBit(pieces);
        int index = _pext_u64(occupied, ROOK_MASK[pos]);
        score += rookMobility[countSetBits(ROOK_ATTACKS[pos][index] & safeSquares)];
    }

    pieces = board.getPieceBitboard(color, QUEEN);
    while (pieces) {
        int pos = popBit(pieces);
        int index1 = _pext_u64(occupied, BISHOP_MASK[pos]);
        int index2 = _pext_u64(occupied, ROOK_MASK[pos]);
        score += queenMobility[countSetBits((BISHOP_ATTACKS[pos][index1] | ROOK_ATTACKS[pos][index2]) & safeSquares)];
    }

    return score;
}

// calculate this incrementally in make/unmake
int positionalScore(Board& board, Color color, bool endgame){
    const int (*squareTable)[64] = endgame ? endgamePieceSquareTable : pieceSquareTable;
    int score = 0;
    uint64_t pieces = board.getColorBitboard(color);
    while (pieces) {
        int pos = popBit(pieces);
        PieceType pieceType = type_of(board.getPieceOnSquare(pos));
        if (color==WHITE) score += squareTable[pieceType][mirrorTable[pos]];
        else score += squareTable[pieceType][pos];
    }
    return score;
}


int rookOpenFile(Board& board) {
    int score = 0;
    uint64_t rooks = board.getPieceBitboard(BLACK,ROOK) | board.getPieceBitboard(WHITE, ROOK);
    const uint64_t pawns[2] = {board.getPieceBitboard(WHITE,PAWN), board.getPieceBitboard(BLACK,PAWN)};
    while (rooks) {
        int pos = popBit(rooks);
        Color color = color_of(board.getPieceOnSquare(pos));
        Color otherColor = color==WHITE?BLACK:WHITE;
        int column = pos%8;
        if ((BOARD_COLUMNS[column]&pawns[color])==0) {
            if ((BOARD_COLUMNS[column]&pawns[otherColor])==0) {
                if (color==WHITE) score += 12;
                else score -= 12;
            } else {
                if (color==WHITE) score += 6;
                else score -= 6;
            }
        }
    }
    return score;
}

int pawnStructure(Board& board) {
    int score = 0;

    int pawnsOnColumn[2][8];
    const uint64_t wPawns = board.getPieceBitboard(WHITE,PAWN);
    const uint64_t bPawns = board.getPieceBitboard(BLACK,PAWN);
    for (int i=0;i<8;i++){
        pawnsOnColumn[0][i] = countSetBits(wPawns & BOARD_COLUMNS[i]);
        pawnsOnColumn[1][i] = countSetBits(bPawns & BOARD_COLUMNS[i]);

        // doubled pawn
        if (pawnsOnColumn[0][i]>1) score += -20 * (pawnsOnColumn[0][i]-1);
        if (pawnsOnColumn[1][i]>1) score += 20 * (pawnsOnColumn[1][i]-1);

        // isolated pawns
        if (pawnsOnColumn[0][i]!=0) {
            if ((wPawns & NEIGHBORING_COLUMNS[i]) == 0)
                score -= 15;
        }
        if (pawnsOnColumn[1][i]!=0) {
            if ((bPawns & NEIGHBORING_COLUMNS[i]) == 0)
                score += 15;
        }

    }
    return score;
}

int passedPawns(Board& board, Color color) {
    int score = 0;
    Color otherColor = color==WHITE?BLACK:WHITE;
    uint64_t pawns = board.getPieceBitboard(color, PAWN);
    static constexpr int passedPawnValue[8] = {0, 7, 10, 15, 22, 33, 60,0};
    while (pawns) {
        int pos = popBit(pawns);
        if ((PASSED_PAWNS[color][pos]&board.getPieceBitboard(otherColor,PAWN))==0) {
            if ((BOARD_RAYS[color==WHITE?0:4][pos]&board.getPieceBitboard(color,PAWN))==0) {
                if (color==WHITE) score += passedPawnValue[pos/8];
                else score += passedPawnValue[7-pos/8];
            }
        }
    }
    return score;
}


int evaluatePosition(Board& board){
    int score = 0;
    int openingScore = 0;
    int endgameScore = 0;
    score += board.getMaterialScore();
    score += pawnStructure(board);
    if (countSetBits(board.getPieceBitboard(WHITE, BISHOP))==2) score += 40;
    if (countSetBits(board.getPieceBitboard(BLACK, BISHOP))==2) score -= 40;

    int passedPawnScore = passedPawns(board,WHITE)-passedPawns(board,BLACK);
    openingScore += passedPawnScore;
    endgameScore += 2*passedPawnScore;

    openingScore += positionalScore(board, WHITE)-positionalScore(board, BLACK);
    endgameScore += positionalScore(board, WHITE, true) - positionalScore(board, BLACK, true);

    int rook = rookOpenFile(board);
    openingScore += rook;
    endgameScore += rook * 3 / 2;

    int mobility = mobilityScore(board,WHITE)-mobilityScore(board,BLACK);
    openingScore += mobility;
    endgameScore += mobility;


    openingScore += score;
    endgameScore += score;
    score = (openingScore*board.getGamePhase()+endgameScore*(32-board.getGamePhase()))/32;
    return board.getSideToMove()==WHITE?score:-score;
}
