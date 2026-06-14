#include "nnue.h"
#include <fstream>
#include <iostream>
#include "helperFunctions.h"
#include <immintrin.h>

int16_t L0W[768][HIDDEN_LAYER];
int16_t L0B[HIDDEN_LAYER];
int16_t L1W[2*HIDDEN_LAYER];
int16_t L1B;

void loadNNUE(std::string filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open NNUE file\n";
        std::exit(1);
    }
    file.read(reinterpret_cast<char*>(L0W), sizeof(L0W));
    file.read(reinterpret_cast<char*>(L0B), sizeof(L0B));
    file.read(reinterpret_cast<char*>(L1W), sizeof(L1W));
    int16_t l1bBlock[32];
    file.read(reinterpret_cast<char*>(l1bBlock), sizeof(l1bBlock));
    L1B = l1bBlock[0];
}

inline int index768(Piece piece, int square, Color sideToMove) {
    int color = (color_of(piece) == sideToMove) ? 0 : 6;
    int pieceType = type_of(piece) - 1;
    if (sideToMove==BLACK) {
        int row = square / 8;
        int column = square % 8;
        square = (7-row)*8+column;
    }
    return (color + pieceType) * 64 + square;
}

void calculateAccumulator(const Board& board, Accumulator& A) {
    for (int i = 0; i < HIDDEN_LAYER; i++) {
        A.white[i] = L0B[i];
        A.black[i] = L0B[i];
    }
    for (int i = 0; i < 64; i++) {

        Piece piece = board.getPieceOnSquare(i);
        if (piece == EMPTY) continue;

        int wFeature = index768(piece, i, WHITE);
        int bFeature = index768(piece, i, BLACK);

        for (int j = 0; j < HIDDEN_LAYER; j++) {
            A.white[j] += L0W[wFeature][j];
            A.black[j] += L0W[bFeature][j];
        }
    }
}

// takes in accumulator for one side and the weights.
// applies SCReLU to accumulator values and multiplies with the weight and then sums everything
int64_t screlu_weight(const int16_t* acc, const int16_t* weights) {
    const __m256i zero = _mm256_setzero_si256();
    const __m256i qa = _mm256_set1_epi16(QA);
    __m256i sum = _mm256_setzero_si256();

    for (int i = 0; i < HIDDEN_LAYER; i += 16) {
        __m256i CReLU = _mm256_min_epi16(_mm256_max_epi16(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(&acc[i])), zero), qa);
        __m256i weight = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&weights[i]));
        __m256i CReLU_weight = _mm256_mullo_epi16(weight, CReLU);
        __m256i x = _mm256_madd_epi16(CReLU_weight, CReLU);
        sum = _mm256_add_epi32(sum, x);
    }
    int32_t sumParts[8];
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(sumParts), sum);
    int64_t sumFinal = 0;
    for (int i=0;i<8;i++) sumFinal += sumParts[i];
    return sumFinal;
}

int evaluateNNUE(const Accumulator& A, Color sideToMove) {
    const int16_t* movingA = (sideToMove == WHITE) ? A.white : A.black;
    const int16_t* otherA = (sideToMove == WHITE) ? A.black : A.white;
    int64_t sum = 0;
    sum += screlu_weight(movingA, L1W);
    sum += screlu_weight(otherA, L1W + HIDDEN_LAYER);
    sum += static_cast<int64_t>(QA) * L1B;
    return static_cast<int>((sum * SCALE / (static_cast<int64_t>(QA) * QA * QB)));
}

inline void addFeature(Accumulator& acc, Piece piece, int square) {
    int w = index768(piece, square, WHITE);
    int b = index768(piece, square, BLACK);
    // adds 16 weights at once to the accumulator using SIMD intrinsics
    for (int i = 0; i < HIDDEN_LAYER; i += 16) {
        __m256i w_acc = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&acc.white[i]));
        __m256i b_acc = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&acc.black[i]));
        __m256i w_weights = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&L0W[w][i]));
        __m256i b_weights = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&L0W[b][i]));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(&acc.white[i]), _mm256_add_epi16(w_acc, w_weights));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(&acc.black[i]), _mm256_add_epi16(b_acc, b_weights));
    }

}

inline void removeFeature(Accumulator& acc, Piece piece, int square) {
    int w = index768(piece, square, WHITE);
    int b = index768(piece, square, BLACK);
    for (int i = 0; i < HIDDEN_LAYER; i++) {
        acc.white[i] -= L0W[w][i];
        acc.black[i] -= L0W[b][i];
    }
}

// Updates accumulator for a pseudo legal move.
void addMoveToAccumulator(Accumulator& A, const Board& board, Move move) {

    int source = move.sourceSquare();
    int target = move.targetSquare();
    Piece movingPiece = board.getPieceOnSquare(source);
    Color side = color_of(movingPiece);
    Color otherSide = side==WHITE?BLACK:WHITE;

    if (move.moveType() == NORMAL) {
        Piece capturedPiece = board.getPieceOnSquare(target);
        removeFeature(A, movingPiece, source);
        if (capturedPiece != EMPTY) removeFeature(A, capturedPiece, target);
        addFeature(A, movingPiece, target);
        return;
    }

    if (move.moveType() == CASTLE) {
        removeFeature(A, movingPiece, source);
        addFeature(A, movingPiece, target);
        Piece rook = get_piece(side, ROOK);
        int rookSource, rookTarget;
        switch (target) {
            case G1: 
                rookSource = H1; 
                rookTarget = F1; 
                break;
            case C1: 
                rookSource = A1; 
                rookTarget = D1; 
                break;
            case G8: 
                rookSource = H8; 
                rookTarget = F8; 
                break;
            case C8:  
                rookSource = A8; 
                rookTarget = D8; 
                break;
            default: break;
        }
        removeFeature(A, rook, rookSource);
        addFeature(A, rook, rookTarget);
        return;
    }

    if (move.moveType() == PROMOTION) {
        Piece capturedPiece = board.getPieceOnSquare(target);
        Piece promotedPiece = get_piece(side, static_cast<PieceType>(move.pawnPromotion() + 2));
        removeFeature(A, movingPiece, source);
        if (capturedPiece != EMPTY) removeFeature(A, capturedPiece, target);
        addFeature(A, promotedPiece, target);
        return;
    }

    if (move.moveType() == EN_PASSANT) {
        int square = (side == WHITE) ? target + SOUTH : target + NORTH;
        removeFeature(A, movingPiece, source);
        removeFeature(A, get_piece(otherSide, PAWN), square);
        addFeature(A, movingPiece, target);
        return;
    }

}
