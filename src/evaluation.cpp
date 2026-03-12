#include "evaluation.h"
#include "board.h"
#include "constants.h"
#include "helperFunctions.h"
#include "precalculations.h"


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

int pawnStructure(Board& board) {
    int score = 0;

    // doubled pawns
    for (int i=0;i<8;i++){
        if (countSetBits(board.getPieceBitboard(WHITE, PAWN) & BOARD_COLUMNS[i])>1) score += -25;
        if (countSetBits(board.getPieceBitboard(BLACK, PAWN) & BOARD_COLUMNS[i])>1) score += 25;
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


    openingScore += score;
    endgameScore += score;
    score = (openingScore*board.getGamePhase()+endgameScore*(32-board.getGamePhase()))/32;
    return board.getSideToMove()==WHITE?score:-score;
}
