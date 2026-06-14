#include "helperFunctions.h"
#include <iostream>
#include "board.h"
#include "constants.h"
#include <string>
#include <random>

void printBitboard(uint64_t bitboard){
    std::cout << "\n";
    for (int i=0; i<8; i++){
        std::cout << 8-i << "   ";
        for (int j=0; j<8; j++){
            if ((bitboard & 1ULL << ((7-i)*8+j) ? 1 : 0)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n    A B C D E F G H \n";
}

std::string squareToName(uint square){
    std::string result;
    int row = square/8;
    int column = square%8;
    char charColumn = 'a'+column;
    char charRow = '0' + row + 1;
    result += charColumn;
    result += charRow;
    return result;
}

int nameToSquare(std::string input){
    int column = input[0]-'a';
    int row = input[1]-'1';
    return row*8+column;
}


std::string moveToString(Move move){
    std::string result;
    result = squareToName(move.sourceSquare()) + squareToName(move.targetSquare());
    if (move.moveType()== PROMOTION){
        if (move.pawnPromotion()==P_QUEEN) result += "q";
        if (move.pawnPromotion()==P_ROOK) result += "r";
        if (move.pawnPromotion()==P_BISHOP) result += "b";
        if (move.pawnPromotion()==P_KNIGHT) result += "n";
    }
    return result;
}

std::string boardToFen(const Board& board) {
    static constexpr char pieceChars[2][7] = {
        {'-', 'P', 'N', 'B', 'R', 'Q', 'K'},
        {'-', 'p', 'n', 'b', 'r', 'q', 'k'}
    };

    std::string fen;

    for (int i=7; i>=0; i--) {
        int empty = 0;
        for (int j=0; j<8; j++) {
            Piece piece = board.getPieceOnSquare(i*8+j);
            if (piece!=EMPTY) {
                if (empty>0) {
                    fen += static_cast<char>('0' + empty);
                    empty = 0;
                }
                fen += pieceChars[color_of(piece)][type_of(piece)];
                continue;
            }
            empty++;
        }
        if (empty>0) fen += std::to_string(empty);
        if (i>0) fen += '/';
    }

    BoardState state = board.getBoardState();
    fen += ' ';
    fen += (board.getSideToMove() == WHITE) ? 'w' : 'b';

    fen += ' ';

    int castling = state.castlingRights();
    if (castling == 0) {
        fen += '-';
    } else {
        if (castling & 0b1000) fen += 'K';
        if (castling & 0b0100) fen += 'Q';
        if (castling & 0b0010) fen += 'k';
        if (castling & 0b0001) fen += 'q';
    }

    fen += ' ';
    fen += (state.enPassantSquare() == 0) ? "-" : squareToName(state.enPassantSquare());
    fen += ' ';
    fen += std::to_string(state.halfmoveClock());
    fen += ' ';
    fen += std::to_string(board.getPly() / 2 + 1);

    return fen;
}

uint64_t randomInt64(){
    static std::mt19937 rnd = [](){
        std::random_device rd;
        return std::mt19937(rd());
    }();
    static std::uniform_int_distribution<unsigned long long> dist( std::numeric_limits<std::uint64_t>::min(), std::numeric_limits<std::uint64_t>::max());
    return dist(rnd);
}
