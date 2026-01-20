#include "Search.h"

#include <algorithm>
#include <limits>

#include "MoveGen.h"
#include "TranspositionTable.h"

namespace {
const int kCheckmateScore = 100000;
const int kTimeOutScore = 200000;
const int kMateThreshold = 99000;

TranspositionTable g_tt(1 << 20);
int g_current_ply = 0;
int g_max_plies = -1;
int g_repetition_count = 0;

bool TimeUp(std::chrono::steady_clock::time_point deadline) {
    return std::chrono::steady_clock::now() >= deadline;
}

int ToTTScore(int score, int ply) {
    if (score > kMateThreshold) {
        return score + ply;
    }
    if (score < -kMateThreshold) {
        return score - ply;
    }
    return score;
}

int FromTTScore(int score, int ply) {
    if (score > kMateThreshold) {
        return score - ply;
    }
    if (score < -kMateThreshold) {
        return score + ply;
    }
    return score;
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

int MaterialScore(const Board& board) {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        char piece = board.PieceAt(i);
        if (piece == '.' || piece == '\0') {
            continue;
        }
        int value = PieceValue(piece);
        if (piece >= 'A' && piece <= 'Z') {
            score += value;
        } else {
            score -= value;
        }
    }
    return score;
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

bool IsPassedPawn(const Board& board, int index, char pawn) {
    int file = index % 8;
    int rank = index / 8;
    int start = pawn == 'P' ? rank + 1 : rank - 1;
    int end = pawn == 'P' ? 7 : 0;
    int step = pawn == 'P' ? 1 : -1;
    char enemy_pawn = pawn == 'P' ? 'p' : 'P';

    for (int r = start; pawn == 'P' ? r <= end : r >= end; r += step) {
        for (int df = -1; df <= 1; ++df) {
            int f = file + df;
            if (f < 0 || f > 7) {
                continue;
            }
            int sq = r * 8 + f;
            if (board.PieceAt(sq) == enemy_pawn) {
                return false;
            }
        }
    }
    return true;
}

int PassedPawnBonus(const Board& board, int index, char pawn) {
    if (!IsPassedPawn(board, index, pawn)) {
        return 0;
    }
    int rank = index / 8;
    int advance = pawn == 'P' ? rank : (7 - rank);
    return 20 + advance * 4;
}

int RookActivityBonus(int index, char rook) {
    int rank = index / 8;
    if (rook == 'R') {
        return rank == 6 ? 15 : 0;
    }
    return rank == 1 ? -15 : 0;
}

int GamePhase(const Board& board) {
    int phase = 0;
    for (int i = 0; i < 64; ++i) {
        char piece = board.PieceAt(i);
        switch (piece) {
            case 'N':
            case 'n':
            case 'B':
            case 'b':
                phase += 1;
                break;
            case 'R':
            case 'r':
                phase += 2;
                break;
            case 'Q':
            case 'q':
                phase += 4;
                break;
            default:
                break;
        }
    }
    if (phase > 24) {
        phase = 24;
    }
    return phase;
}

int Evaluate(const Board& board) {
    int score = 0;
    int phase = GamePhase(board);
    for (int i = 0; i < 64; ++i) {
        char piece = board.PieceAt(i);
        if (piece == '.' || piece == '\0') {
            continue;
        }
        if (piece >= 'A' && piece <= 'Z') {
            int material = PieceValue(piece);
            score += material;
            if (piece == 'K') {
                int mg = kKingTable[i];
                int eg = -kKingTable[MirrorIndex(i)];
                score += (mg * phase + eg * (24 - phase)) / 24;
            } else {
                score += PieceSquareValue(piece, i);
            }
            score += DevelopmentBonus(piece, i);
            if (piece == 'P') {
                score += PassedPawnBonus(board, i, piece);
            } else if (piece == 'R') {
                score += RookActivityBonus(i, piece);
            }
        } else {
            int mirrored = MirrorIndex(i);
            int material = PieceValue(piece);
            score -= material;
            char upper = static_cast<char>(piece - ('a' - 'A'));
            if (upper == 'K') {
                int mg = kKingTable[mirrored];
                int eg = -kKingTable[i];
                score -= (mg * phase + eg * (24 - phase)) / 24;
            } else {
                score -= PieceSquareValue(upper, mirrored);
            }
            score += DevelopmentBonus(piece, i);
            if (piece == 'p') {
                score -= PassedPawnBonus(board, i, piece);
            } else if (piece == 'r') {
                score += RookActivityBonus(i, piece);
            }
        }
    }
    return board.SideToMove() == 'w' ? score : -score;
}

int DrawScore(const Board& board) {
    int material = MaterialScore(board);
    int side_material = board.SideToMove() == 'w' ? material : -material;
    if (side_material > 0) {
        return -15;
    }
    return 0;
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

int CaptureValue(const Board& board, const Move& move) {
    char target = board.PieceAt(move.to());
    if (target == '.') {
        char moved = board.PieceAt(move.from());
        if ((moved == 'P' || moved == 'p') && move.to() == board.EnPassantSquare()) {
            return PieceValue('p');
        }
        return 0;
    }
    return PieceValue(target);
}

int MoveScore(const Board& board, const Move& move, const Move* preferred) {
    if (preferred != nullptr && move.ToUci() == preferred->ToUci()) {
        return 100000;
    }
    if (move.promotion().has_value()) {
        return 3000 + PieceValue(move.promotion().value());
    }
    if (IsCaptureMove(board, move)) {
        int captured = CaptureValue(board, move);
        int attacker = PieceValue(board.PieceAt(move.from()));
        return 2000 + (captured * 10 - attacker);
    }
    return 0;
}

void OrderMoves(const Board& board, std::vector<Move>& moves, const Move* preferred) {
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return MoveScore(board, a, preferred) > MoveScore(board, b, preferred);
    });
}

int Quiescence(Board& board,
               int alpha,
               int beta,
               int ply,
               std::chrono::steady_clock::time_point deadline,
               uint64_t& qnodes) {
    qnodes += 1;
    if (TimeUp(deadline)) {
        return kTimeOutScore;
    }
    if (board.HalfmoveClock() >= 100 ||
        (g_max_plies >= 0 && g_current_ply + ply >= g_max_plies) ||
        g_repetition_count >= 3) {
        return DrawScore(board);
    }

    int alpha_orig = alpha;
    uint64_t key = board.Hash();
    Move tt_move(0, 0);
    int tt_score = 0;
    if (g_tt.Probe(key, 0, ToTTScore(alpha, ply), ToTTScore(beta, ply), tt_score, tt_move)) {
        return FromTTScore(tt_score, ply);
    }

    int stand_pat = Evaluate(board);
    if (stand_pat >= beta) {
        return beta;
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    auto moves = GenerateLegalMoves(board);
    Move hint(0, 0);
    const Move* hint_ptr = g_tt.PeekBestMove(key, hint) ? &hint : nullptr;
    OrderMoves(board, moves, hint_ptr);
    Move best_move(0, 0);
    for (const auto& move : moves) {
        if (!IsCaptureMove(board, move)) {
            continue;
        }
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Quiescence(board, -beta, -alpha, ply + 1, deadline, qnodes);
        UndoMoveApply(board, undo);

        if (score == -kTimeOutScore) {
            return kTimeOutScore;
        }
        if (score >= beta) {
            g_tt.Store(key, 0, ToTTScore(score, ply), Bound::LOWER, &move);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            best_move = move;
        }
    }

    Bound bound = (alpha <= alpha_orig) ? Bound::UPPER : Bound::EXACT;
    if (best_move.from() != best_move.to()) {
        g_tt.Store(key, 0, ToTTScore(alpha, ply), bound, &best_move);
    } else {
        g_tt.Store(key, 0, ToTTScore(alpha, ply), bound, nullptr);
    }

    return alpha;
}

int Negamax(Board& board,
            int depth,
            int ply,
            int alpha,
            int beta,
            std::chrono::steady_clock::time_point deadline,
            uint64_t& nodes,
            uint64_t& qnodes) {
    nodes += 1;
    if (TimeUp(deadline)) {
        return kTimeOutScore;
    }
    if (board.HalfmoveClock() >= 100 ||
        (g_max_plies >= 0 && g_current_ply + ply >= g_max_plies) ||
        g_repetition_count >= 3) {
        return DrawScore(board);
    }
    bool in_check = InCheck(board, board.SideToMove() == 'w' ? Color::White : Color::Black);
    if (depth == 0 && !in_check) {
        return Quiescence(board, alpha, beta, ply, deadline, qnodes);
    }
    if (in_check && depth > 0) {
        depth += 1;
    }

    int alpha_orig = alpha;
    uint64_t key = board.Hash();
    Move tt_move(0, 0);
    int tt_score = 0;
    if (g_tt.Probe(key, depth, ToTTScore(alpha, ply), ToTTScore(beta, ply), tt_score, tt_move)) {
        return FromTTScore(tt_score, ply);
    }

    auto moves = GenerateLegalMoves(board);
    const Move* tt_ptr = g_tt.PeekBestMove(key, tt_move) ? &tt_move : nullptr;
    OrderMoves(board, moves, tt_ptr);
    if (moves.empty()) {
        if (InCheck(board, board.SideToMove() == 'w' ? Color::White : Color::Black)) {
            return -kCheckmateScore + ply;
        }
        return 0;
    }

    int best = std::numeric_limits<int>::min();
    Move best_move(0, 0);
    for (const auto& move : moves) {
        MoveUndo undo = ApplyMove(board, move);
        board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
        int score = -Negamax(board, depth - 1, ply + 1, -beta, -alpha, deadline, nodes, qnodes);
        UndoMoveApply(board, undo);

        if (score == -kTimeOutScore) {
            return kTimeOutScore;
        }
        if (score > best) {
            best = score;
            best_move = move;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            g_tt.Store(key, depth, ToTTScore(score, ply), Bound::LOWER, &move);
            break;
        }
    }

    if (best != std::numeric_limits<int>::min()) {
        Bound bound = (best <= alpha_orig) ? Bound::UPPER : (best >= beta ? Bound::LOWER : Bound::EXACT);
        if (best != kTimeOutScore) {
            if (best_move.from() != best_move.to()) {
                g_tt.Store(key, depth, ToTTScore(best, ply), bound, &best_move);
            } else {
                g_tt.Store(key, depth, ToTTScore(best, ply), bound, nullptr);
            }
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
    OrderMoves(board, moves, nullptr);
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
                             1,
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
                        uint64_t& outQNodes,
                        bool& outDepth1Completed,
                        int& outTimedOutDepth,
                        int& outRootMoveCount) {
    outNodes = 0;
    outQNodes = 0;
    outDepth = 0;
    outDepth1Completed = false;
    outTimedOutDepth = -1;
    outRootMoveCount = 0;
    g_repetition_count = 0;
    int best_score = 0;
    Move best_move(0, 0);

    for (int depth = 1; depth <= maxDepth; ++depth) {
        auto moves = GenerateLegalMoves(board);
        OrderMoves(board, moves, nullptr);
        if (depth == 1) {
            outRootMoveCount = static_cast<int>(moves.size());
        }
        if (moves.empty()) {
            break;
        }
        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max();
        int local_best = std::numeric_limits<int>::min();
        Move local_best_move = moves.front();
        bool timed_out = false;
        int evaluated_moves = 0;

        for (const auto& move : moves) {
            MoveUndo undo = ApplyMove(board, move);
            board.SetSideToMove(undo.side_to_move == 'w' ? 'b' : 'w');
            int score = -Negamax(board, depth - 1, 1, -beta, -alpha, deadline, outNodes, outQNodes);
            UndoMoveApply(board, undo);

            if (score == -kTimeOutScore) {
                timed_out = true;
                break;
            }

            evaluated_moves += 1;
            if (score > local_best) {
                local_best = score;
                local_best_move = move;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        if (timed_out) {
            outTimedOutDepth = depth;
            if (depth == 1 && evaluated_moves > 0) {
                best_score = local_best;
                best_move = local_best_move;
                outDepth = 1;
            }
            break;
        }

        best_score = local_best;
        best_move = local_best_move;
        outDepth = depth;
        if (depth == 1) {
            outDepth1Completed = true;
        }
    }

    if (outDepth > 0) {
        outBestMove = best_move;
    }
    return best_score;
}

void SetSearchDrawContext(int currentPly, int maxPlies, int repetitionCount) {
    g_current_ply = currentPly;
    g_max_plies = maxPlies;
    g_repetition_count = repetitionCount;
}
