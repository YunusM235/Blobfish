#include <atomic>
#include <iostream>
#include <string>
#include "board.h"
#include "precalculations.h"
#include "helperFunctions.h"
#include <chrono>
#include <immintrin.h>
#include <csignal>
#include <regex>
#include <random>
#include "nnue.h"
#include "search.h"
#include <filesystem>
#include <fstream>

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

std::atomic<bool> stopGenerating{false};

void sigintHandler(int) {
    stopGenerating = true;
}

std::mt19937 mt{ std::random_device{}()};

// returns the number of positions generated
int generateGame (std::ofstream& file) {
    Board board;
    
    // plays a random opening until a position is found that is not 
    // decided yet and king is not in check
    while (true) {
        board = Board();
        for (int i = 0; i < 8; i++) {
            MoveList movesPseudoLegal, moves;
            board.generateCaptures(movesPseudoLegal);
            board.generateNonCaptures(movesPseudoLegal);
            for (int j = 0; j < movesPseudoLegal.getSize(); j++)
                if (board.isLegal(movesPseudoLegal.getMove(j)))
                    moves.appendMove(movesPseudoLegal.getMove(j));   
            if (moves.getSize() == 0) break;
            board.makeMove(moves.getMove(mt() % moves.getSize()));
        }
        if (std::abs(board.getMaterialScore()) < 300 && !board.kingAttacked(board.getSideToMove())) break;
    }

    // 0 White won, 1 Black won, 2 Draw
    int result;
    std::vector<std::pair<std::string, int>> boardHistory;
    while (true) {
        if (board.isRepetition() || board.getHalfMoveClock()==100) {
            result = 2;
            break;
        }

        MoveList movesPseudoLegal, moves;
        board.generateCaptures(movesPseudoLegal);
        board.generateNonCaptures(movesPseudoLegal);
        for (int j = 0; j < movesPseudoLegal.getSize(); j++)
            if (board.isLegal(movesPseudoLegal.getMove(j)))
                moves.appendMove(movesPseudoLegal.getMove(j)); 
        if (moves.getSize()==0) {
            if (board.kingAttacked(board.getSideToMove())) result = board.getSideToMove()==WHITE?BLACK:WHITE; 
            else result = 2;
            break;
        }

        auto [bestMove, score] = searchDataGeneration(board, 7000);

        bool isCapture = board.getPieceOnSquare(bestMove.targetSquare()) != EMPTY || bestMove.moveType() == EN_PASSANT;
        // Don't record positions where best move is a capture
        if (!isCapture && !board.kingAttacked(board.getSideToMove()) && std::abs(score) <= 3000) {
            boardHistory.push_back({boardToFen(board), board.getSideToMove()==WHITE?score:-score});
        }

        if (score > 3000) {
            result = board.getSideToMove();
            break;
        } else if (score < -3000) {
            result = board.getSideToMove()==WHITE?BLACK:WHITE;
            break;
        }

        board.makeMove(bestMove);
    }

    int positions = 0;
    for (auto& [fen, score] : boardHistory) {
        file << fen << " | " << score << " | " ;
        if (result==0) file << "1.0\n";
        else if (result==1) file << "0.0\n";
        else file << "0.5\n";
        positions++;
    }

    return positions;
}

// generates self play data for nnue training
void generateData (std::string fileName) {
    std::signal(SIGINT, sigintHandler);

    int positions = 0;
    auto start = std::chrono::steady_clock::now();
    std::ofstream file(fileName, std::ios::app);

    while (!stopGenerating) {
        positions += generateGame(file);
        if (positions % 10 == 0) {
            auto now     = std::chrono::steady_clock::now();
            double timeDiff  = std::chrono::duration<double>(now - start).count();
            std::cout << "\rpositions: " << positions<< " | pos/sec: " << positions/timeDiff << "       " << std::flush;
        }
    }
    std::cout << "\r\033[2Kpositions: " << positions << "\033[?25h\n";
}

std::vector<std::string> split(const std::string& input) {
    std::regex re(R"([\t ]+)");
    std::sregex_token_iterator it(input.begin(), input.end(), re, -1);
    std::sregex_token_iterator end;
    return std::vector<std::string>(it, end);
}

int main(int argc, char* argv[]) {

    loadNNUE((std::filesystem::read_symlink("/proc/self/exe").parent_path() / "weights.bin").string());

    hashTable.reserve(TT_SIZE);
    for (int i=0;i<TT_SIZE;i++) {
        hashTable.emplace_back();
    }

    if (argc==3 && std::string(argv[1])=="generate") {
        std::cout << "\033[?25l";
        std::cout << "Blobfish NNUE data generation\n" << "writing to: " 
            << std::string(argv[2]) << "\n" << "-------------------------------------------\n";
        generateData(std::string(argv[2]));
        return 0;
    }

    Board board{};


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

