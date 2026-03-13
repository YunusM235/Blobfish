#include "precalculations.h"
#include "constants.h"
#include "helperFunctions.h"
#include <array>
#include <iostream>
#include <immintrin.h>
#include "cmath"

namespace {

    std::array<uint64_t, 64> calculateKingAttacks(){
        std::array<uint64_t, 64> arr{};
        int row, column;
        for (int i=0; i<64; i++){
            row = i/8;
            column = i%8;
            if (row != 0) setBit(arr[i], i+SOUTH);
            if (row != 7) setBit(arr[i], i+NORTH);
            if (column != 0) setBit(arr[i], i+WEST);
            if (column != 7) setBit(arr[i], i+EAST);
            if (row != 0 && column != 0) setBit(arr[i], i+SOUTH_WEST);
            if (row != 0 && column != 7) setBit(arr[i], i+SOUTH_EAST);
            if (row != 7 && column != 0) setBit(arr[i], i+NORTH_WEST);
            if (row != 7 && column != 7) setBit(arr[i], i+NORTH_EAST);
        }
        return arr;
    }

    std::array<uint64_t, 64> calculateKnightAttacks(){
        std::array<uint64_t, 64> arr{};
        int row, column;
        for (int i=0; i<64; i++){
            row = i/8;
            column = i%8;
            if (row < 6 && column != 7) setBit(arr[i], i+KNIGHT_OFFSET_1);
            if (row != 7 && column < 6) setBit(arr[i], i+KNIGHT_OFFSET_2);
            if (row != 0 && column < 6) setBit(arr[i], i+KNIGHT_OFFSET_3);
            if (row > 1 && column != 7) setBit(arr[i], i+KNIGHT_OFFSET_4);
            if (row > 1 && column != 0) setBit(arr[i], i+KNIGHT_OFFSET_5);
            if (row != 0 && column > 1) setBit(arr[i], i+KNIGHT_OFFSET_6);
            if (row != 7 && column > 1) setBit(arr[i], i+KNIGHT_OFFSET_7);
            if (row < 6 && column != 0) setBit(arr[i], i+KNIGHT_OFFSET_8);
        }
        return arr;
    }

    std::array<uint64_t, 64> calculatePawnAttacks(Color color){
        std::array<uint64_t, 64> arr{};
        int row, column;
        for (int i=0; i<64; i++){
            row = i/8;
            column = i%8;
            if (color == WHITE && row == 7) continue;
            if (color == BLACK && row == 0) continue;

            if (color == WHITE && column != 0) setBit(arr[i], i+NORTH_WEST);
            if (color == WHITE && column != 7) setBit(arr[i], i+NORTH_EAST);
            if (color == BLACK && column != 0) setBit(arr[i], i+SOUTH_WEST);
            if (color == BLACK && column != 7) setBit(arr[i], i+SOUTH_EAST);
        }
        return arr;
    }

    std::array<std::array<uint64_t,64>, 8> calculateRays(){
        std::array<std::array<uint64_t,64>, 8> arr{};
        int row, column, new_row, new_column, count;
        int directions[8] = {NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST};
        for (int i=0;i<64;i++){
            for (int j=0;j<8;j++){
                count = 0;
                row = i/8;
                column = i%8;
                while(true) {
                    new_row = (i+directions[j]*count)/8;
                    new_column = (i+directions[j]*count)%8;
                    if (new_row < 0 || new_row > 7 || new_column < 0 || new_column > 7) break;
                    if (row-new_row < -1 || row-new_row > 1 || column-new_column < -1 || column-new_column > 1) break;
                    if (count != 0) setBit(arr[j][i], i+directions[j]*count);

                    row = new_row;
                    column = new_column;
                    count++;
                }
            }
        }

        return arr;
    }

    std::array<uint64_t, 64> calculateBishopRays(){
        std::array<uint64_t, 64> arr{};
        for (int i=0; i<64; i++){
            arr[i] = BOARD_RAYS[1][i] | BOARD_RAYS[3][i] | BOARD_RAYS[5][i] | BOARD_RAYS[7][i];
        }
        return arr;
    }

    std::array<uint64_t, 64> calculateRookRays(){
        std::array<uint64_t, 64> arr{};
        for (int i=0; i<64; i++){
            arr[i] = BOARD_RAYS[0][i] | BOARD_RAYS[2][i] | BOARD_RAYS[4][i] | BOARD_RAYS[6][i];
        }
        return arr;
    }

    std::array<uint64_t, 64> calculateQueenRays(){
        std::array<uint64_t, 64> arr{};
        for (int i=0; i<64; i++){
            arr[i] = ROOK_RAYS[i] | BISHOP_RAYS[i];
        }
        return arr;
    }

}

const std::array<uint64_t, 64> KING_ATTACKS = calculateKingAttacks();
const std::array<uint64_t, 64> KNIGHT_ATTACKS = calculateKnightAttacks();
const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS = {calculatePawnAttacks(WHITE), calculatePawnAttacks(BLACK)};

const std::array<std::array<uint64_t,64>, 8> BOARD_RAYS = calculateRays();

const std::array<uint64_t, 64> BISHOP_RAYS = calculateBishopRays();
const std::array<uint64_t, 64> ROOK_RAYS = calculateRookRays();
const std::array<uint64_t, 64> QUEEN_RAYS = calculateQueenRays();

uint64_t inside = 0b0000000001111110011111100111111001111110011111100111111000000000;
uint64_t h_column = 0b1000000010000000100000001000000010000000100000001000000010000000;
uint64_t a_column = 0b0000000100000001000000010000000100000001000000010000000100000001;
uint64_t row_1 = 0b0000000000000000000000000000000000000000000000000000000011111111;
uint64_t row_8 = 0b1111111100000000000000000000000000000000000000000000000000000000;

const std::array<uint64_t, 64> BISHOP_MASK = [](){
    std::array<uint64_t, 64> arr{};
    for (int i=0; i<64; i++) {
        arr[i] = BISHOP_RAYS[i] & inside;
    }
    return arr;
}();

const std::array<uint64_t, 64> ROOK_MASK = [](){
    std::array<uint64_t, 64> arr{};
    for (int i=0; i<64; i++) {
        arr[i] = ROOK_RAYS[i];
        if (i%8!=0) arr[i] &= ~a_column;
        if (i%8!=7) arr[i] &= ~h_column;
        if (i/8!=0) arr[i] &= ~row_1;
        if (i/8!=7) arr[i] &= ~row_8;
    }
    return arr;
}();

const std::array<std::array<uint64_t,512>, 64> BISHOP_ATTACKS = [](){
    std::array<std::array<uint64_t,512>, 64> arr{};
    Direction directions[4] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
    for (int i=0; i<64; i++) {
        uint64_t subset = 0;
        do {
            uint64_t attacks = 0;
            for (Direction direction : directions) {
                int pos = i;
                if (direction == NORTH_EAST && (pos%8==7 || pos/8==7)) continue;
                if (direction == NORTH_WEST && (pos%8==0 || pos/8==7)) continue;
                if (direction == SOUTH_EAST && (pos%8==7 || pos/8==0)) continue;
                if (direction == SOUTH_WEST && (pos%8==0 || pos/8==0)) continue;

                while (!getBit(subset, pos)) {
                    pos += direction;
                    setBit(attacks, pos);
                    if (direction == NORTH_EAST && (pos%8==7 || pos/8==7)) break;
                    if (direction == NORTH_WEST && (pos%8==0 || pos/8==7)) break;
                    if (direction == SOUTH_EAST && (pos%8==7 || pos/8==0)) break;
                    if (direction == SOUTH_WEST && (pos%8==0 || pos/8==0)) break;
                }
            }
            arr[i][_pext_u64(subset, BISHOP_MASK[i])] = attacks;
            subset = (subset - BISHOP_MASK[i]) & BISHOP_MASK[i];
        } while (subset);
    }
    return arr;
}();

const std::array<std::array<uint64_t,4096>, 64> ROOK_ATTACKS = [](){
    std::array<std::array<uint64_t,4096>, 64> arr{};
    Direction directions[4] = {NORTH, EAST, SOUTH, WEST};
    for (int i=0; i<64; i++) {
        uint64_t subset = 0;
        do {
            uint64_t attacks = 0;
            for (Direction direction : directions) {
                int pos = i;
                if (direction == NORTH && pos/8==7) continue;
                if (direction == EAST && pos%8==7) continue;
                if (direction == SOUTH && pos/8==0) continue;
                if (direction == WEST && pos%8==0) continue;

                while (!getBit(subset, pos)) {
                    pos += direction;
                    setBit(attacks, pos);
                    if (direction == NORTH && pos/8==7) break;
                    if (direction == EAST && pos%8==7) break;
                    if (direction == SOUTH && pos/8==0) break;
                    if (direction == WEST && pos%8==0) break;
                }
            }
            arr[i][_pext_u64(subset, ROOK_MASK[i])] = attacks;
            subset = (subset - ROOK_MASK[i]) & ROOK_MASK[i];
        } while (subset);
    }
    return arr;
}();


const std::array<uint64_t, 8> BOARD_COLUMNS = [](){
    std::array<uint64_t, 8> arr{};
    for (int i=0;i<8;i++){
        arr[i] = BOARD_RAYS[0][i];
        setBit(arr[i], i);
    }
    return arr;
}();

const std::array<uint64_t,8> NEIGHBORING_COLUMNS = []() {
    std::array<uint64_t, 8> arr{};
    for (int i=0;i<8;i++){
        if (i==0) {
            arr[i] = BOARD_COLUMNS[1];
        } else if (i==7) {
            arr[i] = BOARD_COLUMNS[6];
        } else {
            arr[i] = BOARD_COLUMNS[i-1] & BOARD_COLUMNS[i+1];
        }
    }
    return arr;
}();



const std::array<std::array<uint64_t,64>, 2> PASSED_PAWNS = []() {
    std::array<std::array<uint64_t,64>, 2> arr{};
    for (int i=0;i<64;i++) {
        arr[0][i] += BOARD_RAYS[0][i];
        arr[1][i] += BOARD_RAYS[4][i];
        if (i%8!=0) {
            arr[0][i] += BOARD_RAYS[0][i+WEST];
            arr[1][i] += BOARD_RAYS[4][i+WEST];
        }
        if (i%8!=7) {
            arr[0][i] += BOARD_RAYS[0][i+EAST];
            arr[1][i] += BOARD_RAYS[4][i+EAST];
        }
    }
    return arr;
}();


const std::array<std::array<int,256>, 64> LMR_VALUES= []() {
    std::array<std::array<int,256>, 64> arr;
    for (int i = 1; i < 64; i++){
        for (int j = 1; j < 256; j++) {
            arr[i][j] = static_cast<int>(1 + std::log(i) * std::log(j) / 2);
        }
    }
    return arr;
}();



const std::array<std::array<std::array<uint64_t, 64>, 7>,2> PIECE_ZOBRIST = [](){
    std::array<std::array<std::array<uint64_t, 64>, 7>,2>  arr{};
    for (int c=0;c<2;c++) {
        for (int i=0; i<7;i++){
            for (int j=0; j<64; j++){
                arr[c][i][j] = randomInt64();
            }
        }
    }
    return arr;
}();

const std::array<uint64_t, 16> CASTLING_ZOBRIST = [](){
    std::array<uint64_t, 16> arr{};
    for (int i=0; i<16;i++){
        arr[i] = randomInt64();
    }
    return arr;
}();

const std::array<uint64_t, 8> ENPASSANT_ZOBRIST = [](){
    std::array<uint64_t, 8> arr{};
    for (int i=0; i<8;i++){
        arr[i] = randomInt64();
    }
    return arr;
}();

const uint64_t COLOR_ZOBRIST = randomInt64();

const uint64_t BOARD_ZOBRIST = randomInt64();