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

uint64_t randomInt64(){
    static std::mt19937 rnd = [](){
        std::random_device rd;
        return std::mt19937(rd());
    }();
    static std::uniform_int_distribution<unsigned long long> dist( std::numeric_limits<std::uint64_t>::min(), std::numeric_limits<std::uint64_t>::max());
    return dist(rnd);
}
