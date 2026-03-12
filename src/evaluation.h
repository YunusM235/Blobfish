#pragma once
#include "board.h"

int positionalScore(Board& board, Color color, bool endgame=false);

int pawnStructure(Board& board);

int passedPawns(Board& board);

int evaluatePosition(Board& board);
