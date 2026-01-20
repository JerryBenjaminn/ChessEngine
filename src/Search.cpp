#include "Search.h"

#include <limits>

#include "MoveGen.h"

namespace {
const int kPawnTable[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    5,  10,  10, -20, -20,  10,  10,   5,
    5,  -5, -10,   0,   0, -10,  -5,   5,
    0,   0,   0,  20,  20,   0,   0,   0,
    5,   5,  10,  25,  25,  10,   5,   5,
    10, 10,  20,  30,  30,  20,  10,  10,
    50, 50,  50,  50,  50,  50,  50,  50,
    0,   0,   0,   0,   0,   0,   0,   0,
};

const int kKnightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50,
};

const int kBishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20,
};

const int kRookTable[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    5,  10,  10,  10,  10,  10,  10,   5,
    -5,  0,   0,   0,   0,   0,   0,  -5,
    -5,  0,   0,   0,   0,   0,   0,  -5,
    -5,  0,   0,   0,   0,   0,   0,  -5,
    -5,  0,   0,   0,   0,   0,   0,  -5,
    -5,  0,   0,   0,   0,   0,   0,  -5,
    0,   0,   0,   5,   5,   0,   0,   0,
};

const int kQueenTable[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
    -5,    0,   5,   5,   5,   5,   0,  -5,
    0,     0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20,
};

const int kKingTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20,  20,   0,   0,   0,   0,  20,  20,
    20,  30,  10,   0,   0,  10,  30,  20,
};

int MirrorIndex(int index) {
    int file = index % 8;
    int rank = index / 8;
    int mirrored_rank = 7 - rank;
    return mirrored_rank * 8 + file;
}

int PieceValue(char piece) {
    switch (piece) {
        case 'P':
        case 'p':
            return 100;
        case 'N':
        case 'n':
            return 320;
        case 'B':
        case 'b':
            return 330;
        case 'R':
        case 'r':
            return 500;
        case 'Q':
        case 'q':
            return 900;
        default:
            return 0;
    }
}

int PieceSquareValue(char piece, int index) {
    switch (piece) {
        case 'P':
            return kPawnTable[index];
        case 'N':
            return kKnightTable[index];
        case 'B':
            return kBishopTable[index];
        case 'R':
            return kRookTable[index];
        case 'Q':
            return kQueenTable[index];
        case 'K':
            return kKingTable[index];
        default:
            return 0;
    }
}

int DevelopmentBonus(char piece, int index) {
    if (piece == 'N') {
        return (index == 1 || index == 6) ? 0 : 10;
    }
    if (piece == 'B') {
        return (index == 2 || index == 5) ? 0 : 10;
    }
    if (piece == 'n') {
        return (index == 57 || index == 62) ? 0 : -10;
    }
    if (piece == 'b') {
        return (index == 58 || index == 61) ? 0 : -10;
    }
    return 0;
}

int Evaluate(const Board& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        char piece = board.PieceAt(i);
        if (piece == '.' || piece == '\0') {
            continue;
        }
        if (piece >= 'A' && piece <= 'Z') {
            score += PieceValue(piece);
            score += PieceSquareValue(piece, i);
            score += DevelopmentBonus(piece, i);
        } else {
            int mirrored = MirrorIndex(i);
            score -= PieceValue(piece);
            score -= PieceSquareValue(static_cast<char>(piece - ('a' - 'A')), mirrored);
            score += DevelopmentBonus(piece, i);
        }
    }
    return board.SideToMove() == 'w' ? score : -score;
}

int Negamax(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return Evaluate(board);
    }

    auto moves = GenerateLegalMoves(board);
    if (moves.empty()) {
        if (InCheck(board, board.SideToMove() == 'w' ? Color::White : Color::Black)) {
            return -100000 + (3 - depth);
        }
        return 0;
    }

    int best = std::numeric_limits<int>::min();
    for (const auto& move : moves) {
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UndoMoveApply(board, undo);

        if (score > best) {
            best = score;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }

    return best;
}
}  // namespace

int EvaluateMaterial(const Board& board) {
    return Evaluate(board);
}

int SearchBestMove(Board& board, int depth, Move& outBestMove) {
    auto moves = GenerateLegalMoves(board);
    if (moves.empty() || depth <= 0) {
        return 0;
    }

    int alpha = std::numeric_limits<int>::min() + 1;
    int beta = std::numeric_limits<int>::max();
    int best_score = std::numeric_limits<int>::min();
    outBestMove = moves.front();

    for (const auto& move : moves) {
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UndoMoveApply(board, undo);

        if (score > best_score) {
            best_score = score;
            outBestMove = move;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return best_score;
}
