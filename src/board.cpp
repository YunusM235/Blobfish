#include <functional>
#include <iostream>    
#include "constants.h"
#include "board.h"
#include <string>
#include <vector>
#include <regex>
#include <ctype.h>
#include <map>
#include "precalculations.h"
#include "helperFunctions.h"
#include <immintrin.h>


void MoveList::sortMoves(int scores[256]) {
    if (size<2) return;
    for (int i=1;i<size;i++) {
        Move currentMove = moves[i];
        int currentScore = scores[i];
        int j=i;
        while (j>0 && scores[j-1]>currentScore) {
            moves[j] = moves[j-1];
            scores[j] = scores[j-1];
            j--;
        }
        moves[j] = currentMove;
        scores[j] = currentScore;
    }
}

void Move::print() const{
    std::cout << "\n#######################\n";
    std::cout << "sourceSquare:  " << squareToName(sourceSquare()) << "\n";
    std::cout << "targetSquare:  " << squareToName(targetSquare()) << "\n";
    std::cout << "moveType: " << moveType() << "\n";
    std::cout << "pawnPromotion: " << pawnPromotion() << "\n";
    std::cout << "#######################\n";
}

void MoveList::print() const {
    for (int i=0; i<size; i++) {
        moves[i].print();
    }
}



Board::Board(std::string fen){

    std::regex re(R"([\s/]+)");
    std::sregex_token_iterator it(fen.begin(), fen.end(), re, -1);
    std::sregex_token_iterator end;
    std::vector<std::string> substrings(it, end);

    std::map<char,PieceType> charToPiece = {
        {'r', ROOK},
        {'n', KNIGHT},
        {'b', BISHOP},
        {'q', QUEEN},
        {'k', KING},
        {'p', PAWN},
    };

    int count, square;
    PieceType pieceType;
    Color color;
    for (int i=0;i<8;i++){
        count = 0;
        for (char c : substrings[i]){
            if (isdigit(c)){
                for (int j=0;j<c-'0';j++){
                    pieceOnSquare[8*(7-i)+count] = EMPTY;
                    count++;
                }
                continue;
            }

            square = 8*(7-i)+count;
            if (std::islower(c)) {
                color = BLACK;
            } else {
                color = WHITE;
                c = static_cast<char>(std::tolower(c));
            }
            pieceType = charToPiece[c];
            materialScore += pieceValue[color][pieceType];
            gamePhase += piecePhase[pieceType];
            pieceOnSquare[square] = get_piece(color, pieceType);
            setBit(pieceBitboards[color][pieceType], square);
            setBit(colorBitboards[color], square);
            count++;
        }
    }

    occupiedBitboard = colorBitboards[WHITE] | colorBitboards[BLACK];
    notOccupiedBitboard = ~occupiedBitboard;

    int enPassantSquare = 0;
    if (substrings[10] != "-"){
        enPassantSquare = (substrings[10][0]-'a')+ (substrings[10][1]-'0'-1)*8;
    }



    int castlingRights = 0;
    for (char c : substrings[9]){
        switch(c){
            case 'K':
                castlingRights += 8; // 0b1000 = 8
                break;
            case 'Q':
                castlingRights += 4; // 0b0100 = 4
                break;
            case 'k':
                castlingRights += 2; // 0b0010 = 2
                break;
            case 'q':
                castlingRights += 1; // 0b0001 = 1
                break;
        }
    }
    sideToMove = substrings[8]=="w" ? WHITE : BLACK;
    otherColor = sideToMove==WHITE?BLACK:WHITE;
    boardStateHistory[ply].setBoardState(enPassantSquare, std::stoi(substrings[11]), castlingRights, BOARD_ZOBRIST);
}

bool Board::isRepetition() const {
    for (int i=ply-2;i>=ply-boardStateHistory[ply].halfmoveClock();i-=2) {
        if (boardStateHistory[i].hashValue()==boardStateHistory[ply].hashValue()) return true;
    }
    return false;
}

void Board::printAllBitboards(){
    for (int i=1; i<7; i++){
        std::cout << "-------------------------\n";
        std::cout << "\n        White " << i << "\n";
        printBitboard(pieceBitboards[0][i]);
        std::cout << "-------------------------\n";
    }
    for (int i=1; i<7; i++){
        std::cout << "-------------------------\n";
        std::cout << "\n        Black " << i << "\n";
        printBitboard(pieceBitboards[1][i]);
        std::cout << "-------------------------\n";
    }
    std::cout << "-------------------------\n";
    std::cout << "\n       All white\n";
    printBitboard(colorBitboards[WHITE]);
    std::cout << "-------------------------\n";
    std::cout << "-------------------------\n";
    std::cout << "\n       All black\n";
    printBitboard(colorBitboards[BLACK]);
    std::cout << "-------------------------\n";
    std::cout << "-------------------------\n";
    std::cout << "\n       Occupied\n";
    printBitboard(occupiedBitboard);
    std::cout << "-------------------------\n";
}


Move Board::stringToMove(std::string input){
    std::string firstSquare, secondSquare;
    firstSquare += input[0];
    firstSquare += input[1];
    secondSquare += input[2];
    secondSquare += input[3];
    int sourceSquare = nameToSquare(firstSquare);
    int targetSquare = nameToSquare(secondSquare);
    PromotionType pawnPromotion = P_KNIGHT;
    MoveType moveType = NORMAL;
    if (input.size()==5) { // pawn promotion
        moveType = PROMOTION;
        if (input[4]=='q') pawnPromotion=P_QUEEN;
        if (input[4]=='r') pawnPromotion=P_ROOK;
        if (input[4]=='b') pawnPromotion=P_BISHOP;
        if (input[4]=='n') pawnPromotion=P_KNIGHT;
    }
    if (type_of(pieceOnSquare[sourceSquare])==PAWN && pieceOnSquare[targetSquare]==EMPTY
        && (sourceSquare%8 != targetSquare%8)) moveType = EN_PASSANT;

    if (type_of(pieceOnSquare[sourceSquare])==KING && (sourceSquare-targetSquare==2 || sourceSquare-targetSquare==-2)) moveType = CASTLE;
    return {sourceSquare, targetSquare, moveType, pawnPromotion};
}

void Board::generateKnightMoves(MoveList &moveList, bool captures) {
    uint64_t knights = pieceBitboards[sideToMove][KNIGHT];
    while (knights){
        uint64_t targets;
        int pos = popBit(knights);
        if (captures) {
            targets = KNIGHT_ATTACKS[pos] & colorBitboards[otherColor];
        } else {
            targets = KNIGHT_ATTACKS[pos] & notOccupiedBitboard;
        }
        while (targets) {
            int targetPos = popBit(targets);
            moveList.appendMove({pos, targetPos, NORMAL});
        }
    }
}

void Board::generateKingMoves(MoveList &moveList, bool captures) {
    int pos = firstBit(pieceBitboards[sideToMove][KING]);
    uint64_t targets;
    if (captures) {
        targets = KING_ATTACKS[pos] & colorBitboards[otherColor];
    } else {
        targets = KING_ATTACKS[pos] & notOccupiedBitboard;
    }
    while (targets) {
        int targetPos = popBit(targets);
        moveList.appendMove({pos, targetPos});
    }

    if (captures) return;
    int castlingRights = boardStateHistory[ply].castlingRights();
    if (sideToMove==WHITE) {
        castlingRights = castlingRights >> 2;
    } else {
        castlingRights &= 0b11;
    }
    if ((castlingRights & 0b10) && pieceOnSquare[pos+EAST]==EMPTY && pieceOnSquare[pos+2*EAST]==EMPTY) {
        moveList.appendMove({pos, pos+2*EAST, CASTLE});
    }
    if ((castlingRights & 1) && pieceOnSquare[pos+WEST]==EMPTY && pieceOnSquare[pos+2*WEST]==EMPTY && pieceOnSquare[pos+3*WEST]==EMPTY) {
        moveList.appendMove({pos, pos+2*WEST, CASTLE});
    }
}

void Board::generatePawnMoves(MoveList& moveList, bool captures) {
    uint64_t pawns = pieceBitboards[sideToMove][PAWN];
    int direction = sideToMove==WHITE?NORTH:SOUTH;
    while (pawns) {
        int pos = popBit(pawns);
        if (captures) {
            uint64_t targets = PAWN_ATTACKS[sideToMove][pos] & colorBitboards[otherColor];
            while (targets) {
                int targetPos = popBit(targets);
                if ((targetPos/8==7 && sideToMove== WHITE) || (targetPos/8==0 && sideToMove== BLACK)) {
                    moveList.appendMove({pos, targetPos, PROMOTION, P_KNIGHT});
                    moveList.appendMove({pos, targetPos, PROMOTION, P_BISHOP});
                    moveList.appendMove({pos, targetPos, PROMOTION, P_ROOK});
                    moveList.appendMove({pos, targetPos, PROMOTION, P_QUEEN});
                } else {
                    moveList.appendMove({pos, targetPos});
                }
            }
            continue;
        }
        if (pos/8==(sideToMove==WHITE?6:1) && pieceOnSquare[pos+direction]==EMPTY) {
            moveList.appendMove({pos, pos+direction, PROMOTION, P_KNIGHT});
            moveList.appendMove({pos, pos+direction, PROMOTION, P_BISHOP});
            moveList.appendMove({pos, pos+direction, PROMOTION, P_ROOK});
            moveList.appendMove({pos, pos+direction, PROMOTION, P_QUEEN});
            continue;
        }
        if (pieceOnSquare[pos+direction]==EMPTY) {
            moveList.appendMove({pos, pos+direction});
            if (pos/8==(sideToMove==WHITE?1:6) && pieceOnSquare[pos+2*direction]==EMPTY) {
                moveList.appendMove({pos, pos+2*direction});
            }
        }
    }
    if (!captures) return;
    int enPassantSquare = boardStateHistory[ply].enPassantSquare();
    if (enPassantSquare!=0) {
        pawns = PAWN_ATTACKS[otherColor][enPassantSquare] & pieceBitboards[sideToMove][PAWN];
        while (pawns) {
            moveList.appendMove({popBit(pawns), enPassantSquare, EN_PASSANT});
        }
    }
}

void Board::generateSlidingMoves(MoveList& moveList, PieceType pieceType, bool captures) {
    uint64_t pieces = pieceBitboards[sideToMove][pieceType];
    while (pieces) {
        int pos = popBit(pieces);
        uint64_t attacks=0;
        if (pieceType!=ROOK) {
            int index = _pext_u64(occupiedBitboard, BISHOP_MASK[pos]);
            attacks = BISHOP_ATTACKS[pos][index];
        }
        if (pieceType!=BISHOP) {
            int index = _pext_u64(occupiedBitboard, ROOK_MASK[pos]);
            attacks += ROOK_ATTACKS[pos][index];
        }
        attacks &= ~colorBitboards[sideToMove];
        uint64_t capturesBitboard = attacks & colorBitboards[otherColor];
        attacks -= capturesBitboard;
        if (captures) {
            while (capturesBitboard) {
                int targetPos = popBit(capturesBitboard);
                moveList.appendMove({pos, targetPos});
            }
        } else {
            attacks -= captures;
            while (attacks) {
                int targetPos = popBit(attacks);
                moveList.appendMove({pos, targetPos});
            }
        }
    }
}

void Board::generateCaptures(MoveList& moveList) {
    generateKnightMoves(moveList, true);
    generateKingMoves(moveList, true);
    generateSlidingMoves(moveList, BISHOP, true);
    generateSlidingMoves(moveList, ROOK, true);
    generateSlidingMoves(moveList, QUEEN, true);
    generatePawnMoves(moveList, true);
}

void Board::generateNonCaptures(MoveList& moveList) {
    generateKnightMoves(moveList, false);
    generateKingMoves(moveList, false);
    generateSlidingMoves(moveList, BISHOP, false);
    generateSlidingMoves(moveList, ROOK, false);
    generateSlidingMoves(moveList, QUEEN, false);
    generatePawnMoves(moveList, false);
}

void Board::makeMoveOnBoard(int sourceSquare, int targetSquare){
    PieceType piece = type_of(pieceOnSquare[sourceSquare]);
    PieceType capturedPiece = type_of(pieceOnSquare[targetSquare]);
    clearBit(pieceBitboards[sideToMove][piece], sourceSquare);
    clearBit(colorBitboards[sideToMove], sourceSquare);
    clearBit(occupiedBitboard, sourceSquare);
    setBit(notOccupiedBitboard, sourceSquare);
    pieceOnSquare[sourceSquare] = EMPTY;
    if (capturedPiece != EMPTY_TYPE){
        clearBit(pieceBitboards[otherColor][capturedPiece], targetSquare);
        clearBit(colorBitboards[otherColor], targetSquare);
    }
    setBit(pieceBitboards[sideToMove][piece], targetSquare);
    setBit(colorBitboards[sideToMove], targetSquare);
    setBit(occupiedBitboard, targetSquare);
    clearBit(notOccupiedBitboard, targetSquare);
    pieceOnSquare[targetSquare] = get_piece(sideToMove, piece);

}

void Board::undoMoveOnBoard(int sourceSquare, int targetSquare, PieceType capturedPiece){
    PieceType piece = type_of(pieceOnSquare[targetSquare]);
    clearBit(pieceBitboards[sideToMove][piece], targetSquare);
    clearBit(colorBitboards[sideToMove], targetSquare);
    clearBit(occupiedBitboard, targetSquare);
    setBit(notOccupiedBitboard, targetSquare);
    pieceOnSquare[targetSquare] = EMPTY;
    if (capturedPiece != EMPTY_TYPE) {
        setBit(pieceBitboards[otherColor][capturedPiece], targetSquare);
        setBit(colorBitboards[otherColor], targetSquare);
        setBit(occupiedBitboard, targetSquare);
        clearBit(notOccupiedBitboard, targetSquare);
        pieceOnSquare[targetSquare] = get_piece(otherColor, capturedPiece);
    }
    setBit(pieceBitboards[sideToMove][piece], sourceSquare);
    setBit(colorBitboards[sideToMove], sourceSquare);
    setBit(occupiedBitboard, sourceSquare);
    clearBit(notOccupiedBitboard, sourceSquare);
    pieceOnSquare[sourceSquare] = get_piece(sideToMove, piece);
}

bool Board::isAttacked(int square, Color color) {
    if ((KING_ATTACKS[square] & pieceBitboards[color][KING]) ||
        (KNIGHT_ATTACKS[square] & pieceBitboards[color][KNIGHT]) ||
        PAWN_ATTACKS[color==WHITE?BLACK:WHITE][square] & pieceBitboards[color][PAWN]) return true;
    int index = _pext_u64(occupiedBitboard, BISHOP_MASK[square]);
    if (BISHOP_ATTACKS[square][index] & (pieceBitboards[color][BISHOP]|pieceBitboards[color][QUEEN])) return true;
    index = _pext_u64(occupiedBitboard, ROOK_MASK[square]);
    if (ROOK_ATTACKS[square][index] & (pieceBitboards[color][ROOK]|pieceBitboards[color][QUEEN])) return true;
    return false;
}

bool Board::isLegal(Move move) {
    int sourceSquare = move.sourceSquare();
    int targetSquare = move.targetSquare();

    MoveType moveType = move.moveType();
    if (moveType == NORMAL) {
        PieceType capturedPiece = type_of(pieceOnSquare[targetSquare]);
        makeMoveOnBoard(sourceSquare, targetSquare);
        bool result = !isAttacked(firstBit(pieceBitboards[sideToMove][KING]), otherColor);

        undoMoveOnBoard(sourceSquare, targetSquare, capturedPiece);
        return result;
    }
    if (moveType == CASTLE) {
        if (sourceSquare-targetSquare>0) { // long castle
            return (!(isAttacked(sourceSquare,otherColor) || isAttacked(sourceSquare+WEST, otherColor) || isAttacked(targetSquare, otherColor)));
        } else { // short castle
            return (!(isAttacked(sourceSquare,otherColor) || isAttacked(sourceSquare+EAST, otherColor) || isAttacked(targetSquare, otherColor)));
        }
    }
    if (moveType == PROMOTION) {
        PieceType capturedPiece = type_of(pieceOnSquare[targetSquare]);
        makeMoveOnBoard(sourceSquare, targetSquare);
        bool result = !isAttacked(firstBit(pieceBitboards[sideToMove][KING]), otherColor);
        undoMoveOnBoard(sourceSquare, targetSquare, capturedPiece);
        return result;
    }
    if (moveType == EN_PASSANT) {
        int direction;
        if (sideToMove==WHITE) direction = SOUTH;
        else direction = NORTH;
        int capturedPos = targetSquare+direction;
        pieceOnSquare[capturedPos] = EMPTY;
        clearBit(pieceBitboards[otherColor][PAWN], capturedPos);
        clearBit(occupiedBitboard, capturedPos);
        setBit(notOccupiedBitboard, capturedPos);
        makeMoveOnBoard(sourceSquare, targetSquare);
        bool result = !isAttacked(firstBit(pieceBitboards[sideToMove][KING]), otherColor);
        undoMoveOnBoard(sourceSquare, targetSquare);
        pieceOnSquare[capturedPos] = get_piece(otherColor, PAWN);
        setBit(pieceBitboards[otherColor][PAWN], capturedPos);
        setBit(occupiedBitboard, capturedPos);
        clearBit(notOccupiedBitboard, capturedPos);
        return result;
    }
    return false;
}


void Board::makeMove(Move move){

    BoardState currentBoardState = boardStateHistory[ply];

    uint64_t newHashValue = currentBoardState.hashValue();
    int newEnPassantSquare=0;
    int newHalfmoveClock=currentBoardState.halfmoveClock();
    int newCastlingRights=currentBoardState.castlingRights();

    int sourceSquare = move.sourceSquare();
    int targetSquare = move.targetSquare();
    PieceType movingPiece = type_of(pieceOnSquare[sourceSquare]);
    PieceType capturedPiece = type_of(pieceOnSquare[targetSquare]);

    materialScore += pieceValue[sideToMove][capturedPiece];
    gamePhase -= piecePhase[capturedPiece];

    newHashValue ^= COLOR_ZOBRIST;
    if (currentBoardState.enPassantSquare() != 0) {
        newHashValue ^= ENPASSANT_ZOBRIST[currentBoardState.enPassantSquare() % 8];
    }

    // update halfmove clock
    if (type_of(pieceOnSquare[sourceSquare])==PAWN || getBit(occupiedBitboard, targetSquare)){
        newHalfmoveClock = 0;
    } else {
        newHalfmoveClock++;
    }

    // update en passant square
    if (pieceOnSquare[sourceSquare]==W_PAWN && targetSquare == sourceSquare+2*NORTH){
        if (PAWN_ATTACKS[WHITE][sourceSquare+NORTH] & pieceBitboards[BLACK][PAWN]){
            newEnPassantSquare = sourceSquare+NORTH;
            newHashValue ^= ENPASSANT_ZOBRIST[newEnPassantSquare%8];
        }
    }
    if (pieceOnSquare[sourceSquare]==B_PAWN && targetSquare == sourceSquare+2*SOUTH){
        if (PAWN_ATTACKS[BLACK][sourceSquare+SOUTH] & pieceBitboards[WHITE][PAWN]){
            newEnPassantSquare = sourceSquare+SOUTH;
            newHashValue ^= ENPASSANT_ZOBRIST[newEnPassantSquare%8];
        }
    }


    newHashValue ^= CASTLING_ZOBRIST[currentBoardState.castlingRights()];
    // update castling rights

    if (sourceSquare == A1 || targetSquare == A1) newCastlingRights &= 0b1011;
    if (sourceSquare == H1 || targetSquare == H1) newCastlingRights &= 0b0111;
    if (sourceSquare == E1) newCastlingRights &= 0b0011;

    if (sourceSquare == A8 || targetSquare == A8) newCastlingRights &= 0b1110;
    if (sourceSquare == H8 || targetSquare == H8) newCastlingRights &= 0b1101;
    if (sourceSquare == E8) newCastlingRights &= 0b1100;
    newHashValue ^= CASTLING_ZOBRIST[newCastlingRights];



    newHashValue ^= PIECE_ZOBRIST[sideToMove][movingPiece][sourceSquare];
    newHashValue ^= PIECE_ZOBRIST[sideToMove][movingPiece][targetSquare];
    if (capturedPiece!=EMPTY_TYPE) {
        newHashValue ^= PIECE_ZOBRIST[otherColor][capturedPiece][targetSquare];
    }
    makeMoveOnBoard(sourceSquare, targetSquare);

    // pawn promotion
    if (move.moveType()==PROMOTION){
        PieceType promotion = static_cast<PieceType>(move.pawnPromotion()+2);
        newHashValue ^= PIECE_ZOBRIST[sideToMove][PAWN][targetSquare];
        newHashValue ^= PIECE_ZOBRIST[sideToMove][promotion][targetSquare];
        materialScore += pieceValue[sideToMove][promotion] - pieceValue[sideToMove][PAWN];
        clearBit(pieceBitboards[sideToMove][PAWN], targetSquare);
        pieceOnSquare[targetSquare] = get_piece(sideToMove, promotion);
        setBit(pieceBitboards[sideToMove][promotion], targetSquare);
    }

    // en passant
    if (move.moveType() == EN_PASSANT) {
        int captureSquareEnPassant = sideToMove==WHITE?targetSquare+SOUTH : targetSquare+NORTH;
        materialScore += pieceValue[sideToMove][PAWN];
        newHashValue ^= PIECE_ZOBRIST[otherColor][PAWN][captureSquareEnPassant];
        clearBit(pieceBitboards[otherColor][PAWN], captureSquareEnPassant);
        clearBit(colorBitboards[otherColor], captureSquareEnPassant);
        clearBit(occupiedBitboard, captureSquareEnPassant);
        setBit(notOccupiedBitboard, captureSquareEnPassant);
        pieceOnSquare[captureSquareEnPassant] = EMPTY;
    }

    // castle
    if (move.moveType() == CASTLE) {
        if (targetSquare==G1) {
            makeMoveOnBoard(H1, F1);
            newHashValue ^= PIECE_ZOBRIST[WHITE][ROOK][H1];
            newHashValue ^= PIECE_ZOBRIST[WHITE][ROOK][F1];
        }
        if (targetSquare==C1) {
            makeMoveOnBoard(A1, D1);
            newHashValue ^= PIECE_ZOBRIST[WHITE][ROOK][A1];
            newHashValue ^= PIECE_ZOBRIST[WHITE][ROOK][D1];
        }
        if (targetSquare==G8) {
            makeMoveOnBoard(H8, F8);
            newHashValue ^= PIECE_ZOBRIST[BLACK][ROOK][H8];
            newHashValue ^= PIECE_ZOBRIST[BLACK][ROOK][F8];
        }
        if (targetSquare==C8) {
            makeMoveOnBoard(A8, D8);
            newHashValue ^= PIECE_ZOBRIST[BLACK][ROOK][A8];
            newHashValue ^= PIECE_ZOBRIST[BLACK][ROOK][D8];
        }
    }

    ply++;
    sideToMove = sideToMove==WHITE?BLACK:WHITE;
    otherColor = sideToMove==WHITE?BLACK:WHITE;
    moveHistory[ply] = move;
    capturedPieceHistory[ply] = capturedPiece;

    boardStateHistory[ply] = BoardState(newEnPassantSquare, newHalfmoveClock, newCastlingRights, newHashValue);
}



void Board::undoMove(){
    Move move = moveHistory[ply];
    sideToMove = sideToMove==WHITE?BLACK:WHITE;
    otherColor = sideToMove==WHITE?BLACK:WHITE;
    int sourceSquare = move.sourceSquare();
    int targetSquare = move.targetSquare();
    // pawn promotion

    if (move.moveType()==PROMOTION){
        PieceType promotion = static_cast<PieceType>(move.pawnPromotion()+2);
        materialScore += pieceValue[sideToMove][PAWN] - pieceValue[sideToMove][promotion];
        clearBit(pieceBitboards[sideToMove][promotion], targetSquare);
        setBit(pieceBitboards[sideToMove][PAWN], targetSquare);
        pieceOnSquare[targetSquare] = get_piece(sideToMove, PAWN);
    }

    if (move.moveType()==EN_PASSANT){
        int captureSquareEnPassant = sideToMove == WHITE ? targetSquare+SOUTH : targetSquare+NORTH;
        materialScore -= pieceValue[sideToMove][PAWN];
        setBit(pieceBitboards[otherColor][PAWN], captureSquareEnPassant);
        setBit(colorBitboards[otherColor], captureSquareEnPassant);
        setBit(occupiedBitboard, captureSquareEnPassant);
        clearBit(notOccupiedBitboard, captureSquareEnPassant);
        pieceOnSquare[captureSquareEnPassant] = get_piece(otherColor, PAWN);
    }

    if (move.moveType()== CASTLE){
        if (targetSquare==G1) undoMoveOnBoard(H1, F1);
        if (targetSquare==C1) undoMoveOnBoard(A1, D1);
        if (targetSquare==G8) undoMoveOnBoard(H8, F8);
        if (targetSquare==C8) undoMoveOnBoard(A8, D8);
    }
    materialScore -= pieceValue[sideToMove][capturedPieceHistory[ply]];
    gamePhase += piecePhase[capturedPieceHistory[ply]];
    undoMoveOnBoard(sourceSquare, targetSquare, capturedPieceHistory[ply]);
    ply--;
}

void Board::doNullMove(){
    uint64_t newHashValue = boardStateHistory[ply].hashValue();
    newHashValue ^= COLOR_ZOBRIST;

    if (boardStateHistory[ply].enPassantSquare() != 0) {
        newHashValue ^= ENPASSANT_ZOBRIST[boardStateHistory[ply].enPassantSquare()%8];
    }

    int newHalfmoveClock=boardStateHistory[ply].halfmoveClock();
    int newCastlingRights=boardStateHistory[ply].castlingRights();

    sideToMove = sideToMove==WHITE?BLACK:WHITE;
    otherColor = sideToMove==WHITE?BLACK:WHITE;
    ply++;
    boardStateHistory[ply] = BoardState(0, newHalfmoveClock, newCastlingRights, newHashValue);
}

void Board::undoNullMove(){
    sideToMove = sideToMove==WHITE?BLACK:WHITE;
    otherColor = sideToMove==WHITE?BLACK:WHITE;
    ply--;
}


bool Board::kingAttacked(Color color) {
    return isAttacked(firstBit(pieceBitboards[color][KING]), color==WHITE?BLACK:WHITE);
}
