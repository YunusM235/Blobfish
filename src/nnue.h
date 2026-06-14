#pragma once
#include <cstdint>
#include <string>
#include "board.h"

constexpr int SCALE = 400;
constexpr int QA = 255;
constexpr int QB = 64;
constexpr int HIDDEN_LAYER = 256;

extern int16_t L0W[768][HIDDEN_LAYER];
extern int16_t L0B[HIDDEN_LAYER];
extern int16_t L1W[2*HIDDEN_LAYER];
extern int16_t L1B;

void loadNNUE(std::string filePath);

struct Accumulator {
    int16_t white[HIDDEN_LAYER];
    int16_t black[HIDDEN_LAYER];
};

void calculateAccumulator(const Board& board, Accumulator& acc);
int evaluateNNUE(const Accumulator& A, Color sideToMove);
void addMoveToAccumulator(Accumulator& A, const Board& board, Move move);

struct AccumulatorStack {
    Accumulator stack[1024];
    int index = 0;

    void reset(const Board& board) {
        index = 0;
        calculateAccumulator(board, stack[0]);
    }

    void pop() {
        index--;
    }

    void addMove(const Board& board, Move move) {
        index++;
        stack[index] = stack[index-1];
        addMoveToAccumulator(stack[index], board, move);
        
    }

    int eval(Color sideToMove) {
        return evaluateNNUE(stack[index], sideToMove);
    }

};
