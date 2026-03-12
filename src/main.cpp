#include <atomic>
#include <iostream>
#include <string>
#include "board.h"
#include "precalculations.h"
#include "helperFunctions.h"
#include <chrono>
#include <immintrin.h>
#include <bits/std_thread.h>

#include "evaluation.h"
#include "search.h"

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;

std::vector<hashTableEntry> hashTable;

uint64_t perft(Board& board, int depth){
    MoveList moves{};
    board.generateCaptures(moves);
    board.generateNonCaptures(moves);
    uint64_t nodes = 0;
    if (depth == 1) {
        while (moves.getSize()>0) {
            Move move = moves.popMove();
            if (!board.isLegal(move)) continue;
            nodes++;
        }
        return nodes;
    }
    while (moves.getSize() > 0) {
        Move move = moves.popMove();
        if (!board.isLegal(move)) continue;
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.undoMove();
    }
    return nodes;
}


std::vector<std::string> split(const std::string& input) {
    std::regex re(R"([\t ]+)");
    std::sregex_token_iterator it(input.begin(), input.end(), re, -1);
    std::sregex_token_iterator end;
    return std::vector<std::string>(it, end);
}

int main() {
    Board board{};

    hashTable.reserve(TT_SIZE);
    for (int i=0;i<TT_SIZE;i++) {
        hashTable.emplace_back();
    }

    std::string input;
    std::vector<std::string> substrings;
    int movesLeft = 41;
    while (true) {
        std::getline(std::cin, input);
        std::vector<std::string> str = split(input);

        for (uint i=0; i<str.size(); i++) {
            if (str[i].empty()) continue;
            if (str[i] == "uci") {
                std::cout << "id name Blobfish\n";
                std::cout << "id author YM\n";
                std::cout << "uciok\n";
            } else if (str[i] == "isready") {
                std::cout << "readyok\n";
            } else if (str[i] == "ucinewgame") {
                movesLeft = 41;
                hashTable.assign(TT_SIZE, {});
            } else if (str[i] == "position") {
                if (str[i+1] == "fen") {
                    std::string fen;
                    for (int j=0;j<6;j++){
                        fen += str[i+2+j];
                        fen += " ";
                    }
                    board = Board(fen);
                    i += 8;
                } else if (str[i+1] == "startpos") {
                    board = Board();
                    i += 2;
                } else continue;
                while (i<str.size()-1) {
                    i++;
                    board.makeMove(board.stringToMove(str[i]));
                }

                break;
            } else if (str[i] == "go") {
                int wtime=0, btime=0, increment=0;
                for (uint j=i+1;j<str.size();j++) {
                    if (str[j] == "wtime") {
                        wtime = std::stoi(str[j+1]);
                        j++;
                    } else if (str[j] == "btime") {
                        btime = std::stoi(str[j+1]);
                        j++;
                    }
                    if (str[j] == "winc") {
                        increment = std::stoi(str[j+1]);
                        j++;
                    }
                }
                movesLeft = std::max(20, movesLeft-2);
                int searchTime = board.getSideToMove()==WHITE?wtime:btime;
                if (wtime==0 && btime==0) searchTime = 1000;
                else searchTime = searchTime/movesLeft + increment * 9 /10;
                Move bestMove = searchBestMove(board, searchTime);
                std::cout << "bestmove " << moveToString(bestMove) << "\n";

            } else if (str[i] == "stop") {

            } else if (str[i] == "quit") {
                return 0;
            }

        }

    }
}

