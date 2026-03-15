#pragma once
#include "board.h"

int mobilityScore(Board& board, Color color);

int positionalScore(Board& board, Color color, bool endgame=false);

int pawnStructure(Board& board);

int passedPawns(Board& board);

int evaluatePosition(Board& board);
