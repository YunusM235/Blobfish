#include "search.h"
#include <algorithm>
#include <atomic>
#include <functional>
#include <vector>
#include "board.h"
#include "constants.h"
#include "evaluation.h"
#include "helperFunctions.h"
#include <chrono>
#include <thread>
#include <signal.h>
#include <cstring>
#include "cmath"
#include "precalculations.h"

extern std::vector<hashTableEntry> hashTable;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

std::chrono::time_point<std::chrono::steady_clock> softLimit;
std::chrono::time_point<std::chrono::steady_clock> hardLimit;

int nodes = 0;

Move killerMoves[64][2];
int historyScore[2][64][64];

bool stopSearch;
int rootDepth = 0;

void sortCaptures (const Board& board, MoveList& moves) {
    if (moves.getSize()<2) return;
    int bestScore=MIN;
    int bestPos=0;
    for (int i=0;i<moves.getSize();i++) {
        PieceType moving = type_of(board.getPieceOnSquare(moves.getMove(i).sourceSquare()));
        PieceType captured = type_of(board.getPieceOnSquare(moves.getMove(i).targetSquare()));
        if (MVV_LVA[moving][captured]>bestScore) {
            bestScore = MVV_LVA[moving][captured];
            bestPos = i;
        }
    }
    moves.swapMoves(moves.getSize()-1,bestPos);
}

void sortNonCaptures(const Board& board, MoveList& moves, int depth) {
    Color color = board.getSideToMove();
    for (int i=0;i<moves.getSize();i++) {
        Move move = moves.getMove(i);
        if (move==killerMoves[depth][0] || move==killerMoves[depth][1]) {
            moves.swapMoves(i, moves.getSize()-1);
            return;
        }
    }
    int bestScore=INT_MIN;
    int bestPos=0;
    for (int i=0;i<moves.getSize()-1;i++) {
        Move move = moves.getMove(i);
        int score = historyScore[color][move.sourceSquare()][move.targetSquare()];
        if (score>bestScore) {
            bestScore=score;
            bestPos=i;
        }
    }
    moves.swapMoves(bestPos,moves.getSize()-1);
}


int quiescence(Board& board, int alpha, int beta) {
    nodes++;
    if ((nodes&2047)==0) {
        if (std::chrono::steady_clock::now()>=hardLimit) return 0;
    }
    if (stopSearch) return 0;
    MoveList moves;
    board.generateCaptures(moves);
    int eval = evaluatePosition(board);
    if (eval>=beta) return eval;
    if (board.getGamePhase()>3 && eval + 1000 < alpha) return eval;
    if (eval>alpha) alpha = eval;

    int bestScore=eval;
    while (moves.getSize()) {
        sortCaptures(board, moves);
        Move move = moves.popMove();
        PieceType capturedPiece = type_of(board.getPieceOnSquare(move.targetSquare()));
        if (board.getGamePhase()>3 && eval + pieceValue[0][capturedPiece] + 200 < alpha) continue;
        if (!board.isLegal(move)) continue;
        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha);
        board.undoMove();
        if (score>=beta) return score;
        if (score>bestScore) {
            bestScore=score;
            if (score>alpha) alpha = score;
        }
    }
    return bestScore;
}

int alphaBeta(Board& board, int alpha, int beta, int depth) {
    nodes++;
    if ((nodes&2047)==0) {
        if (std::chrono::steady_clock::now()>=hardLimit) stopSearch=true;
    }
    if (stopSearch) return 0;
    if (board.isRepetition() || board.getHalfMoveClock()==100) return 0;

    if (depth==0) {
        return quiescence(board, alpha, beta);
    }

    int kingSquare = firstBit(board.getPieceBitboard(board.getSideToMove(),KING));
    bool inCheck = board.isAttacked(kingSquare, board.getOtherColor());
    if (inCheck && depth<2*rootDepth) depth++;
    uint32_t hashIndex = board.getHashValue()&(TT_SIZE-1);
    if (hashTable[hashIndex].hash_key==board.getHashValue() && hashTable[hashIndex].depth>=depth) {
        ScoreType scoreType = hashTable[hashIndex].scoreType();
        int score = hashTable[hashIndex].score;
        if (score>MATE) score -= board.getPly();
        if (score<-MATE) score += board.getPly();
        if ((scoreType==EXACT) ||
            (scoreType==LOWER && score>=beta) ||
            (scoreType==UPPER && score<=alpha)) return score;
    }

    if (board.getGamePhase()>3 && depth>=3 && !inCheck) {
        board.doNullMove();
        int score = -alphaBeta(board, -beta, -beta+1, depth-3);
        board.undoNullMove();
        if (score>=beta) {
            return score;
        }
    }

    if (!inCheck && depth <= 7) {
        int eval = evaluatePosition(board);
        if (eval - 120 * depth >= beta) {
            return eval - 120 * depth;
        }
    }

    int oldAlpha = alpha;
    int bestScore=MIN;
    Move bestMove{};

    Move ttMove{};
    if (hashTable[hashIndex].hash_key==board.getHashValue()) {
        ttMove = hashTable[hashIndex].bestMove;
        board.makeMove(ttMove);
        int score = -alphaBeta(board, -beta, -alpha, depth-1);
        board.undoMove();
        if (stopSearch) return 0;
        if (score>=beta) {
            hashTable[hashIndex] = {board.getHashValue(), ttMove, score, depth, LOWER};
            return score;
        }
        if (score>bestScore) {
            bestScore = score;
            bestMove = ttMove;
            if (score>alpha) alpha = score;
        }
    }

    MoveList moves;

    board.generateCaptures(moves);

    int numMoves=moves.getSize();
    MoveList searchedNonCaptures;
    for (int i=0;i<2;i++) {
        if (i==1) {
            board.generateNonCaptures(moves);
            numMoves+=moves.getSize();
        }
        while (moves.getSize()) {
            if (i==0) sortCaptures(board, moves);
            else sortNonCaptures(board, moves, depth);
            Move move = moves.popMove();
            if (move==ttMove) continue;
            if (!board.isLegal(move)) {
                numMoves--;
                continue;
            }
            if (i==1) {
                searchedNonCaptures.appendMove(move);
            }
            board.makeMove(move);
            int score;
            int reducedDepth = depth-1;
            if (i==1 && depth>=3 && searchedNonCaptures.getSize()>=3 && !board.kingAttacked(board.getSideToMove())) {
                int r = std::min(LMR_VALUES[depth][searchedNonCaptures.getSize()],depth-2);
                reducedDepth = depth-1-r;
            }
            if (bestScore==MIN) {
                score = -alphaBeta(board, -beta, -alpha, depth-1);
            } else {
                score = -alphaBeta(board, -alpha-1, -alpha, reducedDepth); // zero window (maybe reduced depth)
                if (score>alpha && reducedDepth!=depth-1) { // zero window full depth (if depth was reduced)
                    score = -alphaBeta(board, -alpha-1, -alpha, depth-1);
                }
                if (score>alpha) { // full window, full depth
                    score = -alphaBeta(board, -beta, -alpha, depth-1);
                }
            }

            board.undoMove();
            if (stopSearch) return 0;
            if (score>=beta) {

                if (i==1) {
                    if (!(move == killerMoves[depth][0])) {
                        killerMoves[depth][1] = killerMoves[depth][0];
                        killerMoves[depth][0] = move;
                    }
                    for (int j=0;j<searchedNonCaptures.getSize()-1;j++) {
                        Move m = searchedNonCaptures.getMove(j);
                        historyScore[board.getSideToMove()][m.sourceSquare()][m.targetSquare()] -= depth*depth;
                    }
                    historyScore[board.getSideToMove()][move.sourceSquare()][move.targetSquare()] += depth*depth;
                }

                int ttScore = score;
                if (ttScore>MATE) ttScore += board.getPly();
                if (ttScore<-MATE) ttScore -= board.getPly();
                hashTable[hashIndex] = {board.getHashValue(), move, ttScore, depth, LOWER};
                return score;
            }
            if (score>bestScore) {
                bestScore = score;
                bestMove = move;
                if (score>alpha) alpha = score;
            }
        }
    }
    if (numMoves==0) {
        if (inCheck){
            return MIN + board.getPly();
        }
        return 0;
    }
    int ttScore = bestScore;
    if (ttScore>MATE) ttScore += board.getPly();
    if (ttScore<-MATE) ttScore -= board.getPly();
    if (stopSearch) return 0;
    hashTable[hashIndex] = {board.getHashValue(), bestMove, ttScore, depth, bestScore>oldAlpha?EXACT:UPPER};
    return bestScore;
}



Move searchBestMove(Board &board, int searchTime) {
    std::memset(historyScore, 0, sizeof(historyScore));
    nodes=0;
    MoveList movesPseudoLegal;
    MoveList moves;
    board.generateNonCaptures(movesPseudoLegal);
    board.generateCaptures(movesPseudoLegal);
    for (int i=0;i<movesPseudoLegal.getSize();i++) {
        if (board.isLegal(movesPseudoLegal.getMove(i))) moves.appendMove(movesPseudoLegal.getMove(i));
    }

    int bestScore=0, currBestScore;
    Move bestMove, currBestMove;
    bestMove = moves.getMove(0);
    if (moves.getSize()==1) return bestMove;
    int softBound = searchTime * 6 / 10;
    int hardBound = searchTime * 4;

    auto start = std::chrono::steady_clock::now();
    softLimit = start + std::chrono::milliseconds(softBound);
    hardLimit = start + std::chrono::milliseconds(hardBound);

    stopSearch = false;
    int scores[256];
    for (int depth=1;depth<64;depth++) {
        nodes=0;
        rootDepth = depth;
        currBestScore = MIN;

        int delta = 20;
        int alpha, beta;
        if (depth <=3) {
            alpha = MIN;
            beta = MAX;
        } else {
            alpha = bestScore - delta;
            beta = bestScore + delta;
        }
        bool failHigh = false;

        while (true) {
            currBestScore = alpha;
            currBestMove = bestMove;
            failHigh = false;
            for (int i=moves.getSize()-1;i>=0;i--) {
                board.makeMove(moves.getMove(i));
                int score;
                if (i==moves.getSize()-1) {
                    score = -alphaBeta(board, -beta, -currBestScore, depth);
                } else {
                    score = -alphaBeta(board, -currBestScore-1, -currBestScore, depth);
                    if (score>currBestScore) {
                        score = -alphaBeta(board, -beta, -currBestScore, depth);
                    }
                }
                board.undoMove();
                scores[i] = score;
                if (score > currBestScore) {
                    currBestScore = score;
                    currBestMove = moves.getMove(i);
                }
                if (score > beta) {
                    failHigh = true;
                    break;
                }
                if (std::chrono::steady_clock::now()>=softLimit) break;
            }
            if (failHigh) {
                delta += delta/2;
                beta = std::min(currBestScore+delta, MAX);
                continue;
            }

            if (currBestScore==alpha) { //fail low
                delta += delta/2;
                alpha = std::max(currBestScore-delta, MIN);
                continue;
            }
            break; // score was in window
        }

        if (std::chrono::steady_clock::now()>=softLimit) break;
        bestScore = currBestScore;
        bestMove = currBestMove;
        moves.sortMoves(scores);
    }

    return bestMove;
}
