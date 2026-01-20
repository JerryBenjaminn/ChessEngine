#include "Search.h"

#include <limits>

#include "MoveGen.h"

namespace {
const int kCheckmateScore = 100000;
const int kTimeOutScore = 200000;

bool TimeUp(std::chrono::steady_clock::time_point deadline) {
    return std::chrono::steady_clock::now() >= deadline;
}
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

bool IsCaptureMove(const Board& board, const Move& move) {
    int to = move.to();
    char target = board.PieceAt(to);
    if (target != '.') {
        return true;
    }
    char moved = board.PieceAt(move.from());
    if ((moved == 'P' || moved == 'p') && to == board.EnPassantSquare()) {
        return true;
    }
    return false;
}

int Quiescence(Board& board,
               int alpha,
               int beta,
               std::chrono::steady_clock::time_point deadline,
               uint64_t& qnodes) {
    if (TimeUp(deadline)) {
        return kTimeOutScore;
    }

    qnodes += 1;
    int stand_pat = Evaluate(board);
    if (stand_pat >= beta) {
        return beta;
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    auto moves = GenerateLegalMoves(board);
    for (const auto& move : moves) {
        if (!IsCaptureMove(board, move)) {
            continue;
        }
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Quiescence(board, -beta, -alpha, deadline, qnodes);
        UndoMoveApply(board, undo);

        if (score == -kTimeOutScore) {
            return kTimeOutScore;
        }
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int Negamax(Board& board,
            int depth,
            int alpha,
            int beta,
            std::chrono::steady_clock::time_point deadline,
            uint64_t& nodes,
            uint64_t& qnodes) {
    if (TimeUp(deadline)) {
        return kTimeOutScore;
    }
    if (depth == 0) {
        nodes += 1;
        return Quiescence(board, alpha, beta, deadline, qnodes);
    }

    auto moves = GenerateLegalMoves(board);
    if (moves.empty()) {
        if (InCheck(board, board.SideToMove() == 'w' ? Color::White : Color::Black)) {
            return -kCheckmateScore + depth;
        }
        return 0;
    }

    int best = std::numeric_limits<int>::min();
    for (const auto& move : moves) {
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Negamax(board, depth - 1, -beta, -alpha, deadline, nodes, qnodes);
        UndoMoveApply(board, undo);

        if (score == -kTimeOutScore) {
            return kTimeOutScore;
        }
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
        uint64_t nodes = 0;
        uint64_t qnodes = 0;
        int score = -Negamax(board,
                             depth - 1,
                             -beta,
                             -alpha,
                             std::chrono::steady_clock::time_point::max(),
                             nodes,
                             qnodes);
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

int SearchBestMoveTimed(Board& board,
                        int maxDepth,
                        std::chrono::steady_clock::time_point deadline,
                        Move& outBestMove,
                        int& outDepth,
                        uint64_t& outNodes,
                        uint64_t& outQNodes) {
    outNodes = 0;
    outQNodes = 0;
    outDepth = 0;
    int best_score = 0;
    Move best_move(0, 0);

    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (TimeUp(deadline)) {
            break;
        }
        auto moves = GenerateLegalMoves(board);
        if (moves.empty()) {
            break;
        }
        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max();
        int local_best = std::numeric_limits<int>::min();
        Move local_best_move = moves.front();
        bool timed_out = false;

        for (const auto& move : moves) {
            MoveUndo undo = ApplyMove(board, move);
            board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            int score = -Negamax(board, depth - 1, -beta, -alpha, deadline, outNodes, outQNodes);
            UndoMoveApply(board, undo);

            if (score == -kTimeOutScore) {
                timed_out = true;
                break;
            }

            if (score > local_best) {
                local_best = score;
                local_best_move = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        if (timed_out) {
            break;
        }

        best_score = local_best;
        best_move = local_best_move;
        outDepth = depth;
    }

    if (outDepth > 0) {
        outBestMove = best_move;
    }
    return best_score;
}
