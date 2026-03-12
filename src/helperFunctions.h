#pragma once
#include <cstdint>
#include <string>
#include "board.h"

inline PieceType type_of(Piece piece) {
    return static_cast<PieceType>(piece & 0b111);
}

inline Color color_of(Piece piece) {
    return static_cast<Color>(piece >> 3);
}

inline Piece get_piece(Color color, PieceType pieceType) {
    return static_cast<Piece>( color<<3 | pieceType);
}


inline bool getBit(const uint64_t& bitboard, int square){
    return bitboard & 1ULL << square;
}

inline void setBit(uint64_t& bitboard, int square){
    bitboard = bitboard | (1ULL << square);
}

inline void clearBit(uint64_t &bitboard, int square) {
    bitboard = bitboard & ~(1ULL << square);
}

inline int firstBit(uint64_t bitboard) {
    if (bitboard==0) return -1;
    return __builtin_ctzll(bitboard);
}

inline int lastBit(uint64_t bitboard) {
    if (bitboard==0) return -1;
    return 63 - __builtin_clzll(bitboard);
}

inline int popBit(uint64_t& bitboard){
    int position = firstBit(bitboard);
    bitboard &= bitboard - 1;
    return position;
}

inline int countSetBits(uint64_t bitboard) {
    return __builtin_popcountll(bitboard);
}

void printBitboard(uint64_t bitboard);

std::string squareToName(uint square);

int nameToSquare(std::string input);

std::string moveToString(Move move);

uint64_t randomInt64();
