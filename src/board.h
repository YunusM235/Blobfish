#pragma once
#include "constants.h"
#include "board.h"
#include <string>
#include <regex>

class Move{

    /*
    First 6 bits are the source square
    next 6 bits are the target square
    next 2 bits are promotion type
    next 2 bits are move type
    */
    uint16_t move = 0;

    public:

    void print() const;

    uint16_t sourceSquare() const {
        return move & 0b111111;
    }
    uint16_t targetSquare() const {
        return (move >> 6) & 0b111111;
    }
    MoveType moveType() const {
        return static_cast<MoveType>(move >> 14);
    }
    PromotionType pawnPromotion() const {
        return static_cast<PromotionType>( move>>12 & 0b11);
    }

    bool operator==(const Move& otherMove) const {
        return this->move==otherMove.move;
    }

    Move(int sourceSquare = 0, int targetSquare = 0, MoveType moveType = NORMAL, PromotionType pawnPromotion = P_KNIGHT) {
        move += sourceSquare;
        move += targetSquare << 6;
        move += pawnPromotion << 12;
        move += moveType << 14;
    }
};


class MoveList {
    Move moves[256] = {};
    int size = 0;

    public:

    void print() const;
    int getSize() const {
        return size;
    }
    Move getMove(int i) const {
        return moves[i];
    }
    void swapMoves(int i, int j) {
        std::swap(moves[i],moves[j]);
    }

    void appendMove(Move move){
        moves[size] = move;
        size++;
    }
    Move popMove() {
        if (size==0) return {};
        size--;
        return moves[size];
    }

    void sortMoves(int scores[256]);

};


class BoardState{

    /*
    first 6 bits are the enPassantSquare (0 if there is not enpassant square)
    next 7 bits are the halfmove clock
    last 4 bits are the castling rights:
        first two bits are blacks castling right and the next two bits are whites castling rights
        1111 -> both sides can castle both ways
        0111 -> white can't castle kingside
        1110 -> black can't castle queenside
    */
    uint32_t boardState = 0;
    uint64_t hash_value = 0;

    public:

    uint16_t enPassantSquare() const {
        return boardState & 0b111111;
    }

    uint16_t halfmoveClock() const {
        return (boardState >> 6) & 0b1111111;
    }

    uint16_t castlingRights() const {
        return (boardState >> 13) & 0b1111;
    }

    uint64_t hashValue() const {
        return hash_value;
    }
    void setBoardState(int enPassantSquare = 0, int halfmoveClock = 0, int castlingRights = 0b1111, uint64_t hashValue = 0) {
        boardState = 0;
        boardState += enPassantSquare;
        boardState += halfmoveClock << 6;
        boardState += castlingRights << 13;
        hash_value = hashValue;
    }
    BoardState(int enPassantSquare = 0, int halfmoveClock = 0, int castlingRights = 0b1111, uint64_t hashValue = 0) {
        setBoardState(enPassantSquare, halfmoveClock, castlingRights, hashValue);
    }
};

class Board {
    
    Color sideToMove;
    Color otherColor;
    int ply = 0;
    Piece pieceOnSquare[64] = {};

    uint64_t pieceBitboards[2][7] = {};
    uint64_t colorBitboards[2] = {};
    uint64_t occupiedBitboard = {};
    uint64_t notOccupiedBitboard = {};

    BoardState boardStateHistory[1024]{};
    Move moveHistory[1024]{};
    PieceType capturedPieceHistory[1024]{};


    int materialScore = 0;
    int openingPositionalScore = 0;
    int endgamePositionalScore = 0;
    //0 to 32
    int gamePhase = 0;

    void makeMoveOnBoard(int sourceSquare, int targetSquare);
    void undoMoveOnBoard(int sourceSquare, int targetSquare, PieceType capturedPiece=EMPTY_TYPE);

    public:
    Piece getPieceOnSquare(int square) const {
        return pieceOnSquare[square];
    }
    uint64_t getPieceBitboard(Color color, PieceType pieceType) const {
        return pieceBitboards[color][pieceType];
    }

    uint64_t getOccupiedBitboard() const {
        return occupiedBitboard;
    }

    uint64_t getColorBitboard(Color color) const {
        return colorBitboards[color];
    }

    Color getSideToMove() const {
        return sideToMove;
    }

    Color getOtherColor() const {
        return otherColor;
    }

    void flipColors() {
        std::swap(sideToMove, otherColor);
    }

    BoardState getBoardState() const {
        return boardStateHistory[ply];
    }

    int getPly() const {
        return ply;
    }

    uint64_t getHashValue() const {
        return boardStateHistory[ply].hashValue();
    }

    int getHalfMoveClock() const {
        return boardStateHistory[ply].halfmoveClock();
    }

    int getMaterialScore() const {
        return materialScore;
    }

    int getGamePhase() const {
        return gamePhase;
    }

    //checks if the current position was already on the board before
    bool isRepetition() const;


    void printAllBitboards();

    Move stringToMove(std::string input);

    // only pseudo-legal moves are generated
    // if captures is true only captures will be generated otherwise only non-captures will be generated
    void generateKnightMoves(MoveList& moveList, bool captures);
    void generateKingMoves(MoveList& moveList, bool captures);
    void generateSlidingMoves(MoveList& moveList, PieceType pieceType, bool captures);
    void generatePawnMoves(MoveList& moveList, bool captures);

    void generateCaptures(MoveList& moveList);
    void generateNonCaptures(MoveList& moveList);

    // checks if square is attacked by "color"
    bool isAttacked(int square, Color color);
    bool isLegal(Move move);

    bool kingAttacked(Color color);

    void makeMove(Move move);
    void undoMove(); // undoes last move

    void doNullMove();
    void undoNullMove();

    Board(std::string fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

};
