#include "Search.h"

#include <limits>

#include "MoveGen.h"

namespace {
int PieceValue(char piece) {
    switch (piece) {
        case 'P':
            return 100;
        case 'N':
            return 320;
        case 'B':
            return 330;
        case 'R':
            return 500;
        case 'Q':
            return 900;
        case 'p':
            return -100;
        case 'n':
            return -320;
        case 'b':
            return -330;
        case 'r':
            return -500;
        case 'q':
            return -900;
        default:
            return 0;
    }
}

int Evaluate(const Board& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        score += PieceValue(board.PieceAt(i));
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
