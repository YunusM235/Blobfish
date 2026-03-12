#pragma once
#include "board.h"
#include <cstdint>
#include "helperFunctions.h"

struct hashTableEntry {
    uint64_t hash_key;
    Move bestMove;
    int16_t score;
    uint8_t depth;
    uint16_t flag;

    int getDepth() const;
    ScoreType scoreType() const {
        return static_cast<ScoreType>(flag & 0b11);
    }

    hashTableEntry(uint64_t hash_key=0, Move bestMove=Move(), int score=0, int depth=0, ScoreType scoreType=EXACT) {
        this->hash_key = hash_key;
        this->bestMove = bestMove;
        this->score = score;
        this->depth = depth;
        flag = 0;
        flag += scoreType;
    }
};

void sortCaptures (const Board& board, MoveList& moves);
void sortNonCaptures(const Board& board, MoveList& moves, int depth);
int quiescence(Board& board, int alpha, int beta);
int alphaBeta(Board& board, int alpha, int beta, int depth);
Move searchBestMove (Board& board, int searchTime);
