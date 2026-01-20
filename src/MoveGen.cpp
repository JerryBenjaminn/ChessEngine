#include "MoveGen.h"

namespace {
bool IsWhitePiece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

bool IsBlackPiece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

bool IsFriendly(char piece, char side) {
    if (side == 'w') {
        return IsWhitePiece(piece);
    }
    return IsBlackPiece(piece);
}

bool IsEnemy(char piece, char side) {
    if (side == 'w') {
        return IsBlackPiece(piece);
    }
    return IsWhitePiece(piece);
}

void AddMove(std::vector<Move>& moves, int from, int to) {
    moves.emplace_back(from, to);
}

void AddSlidingMoves(const Board& board,
                     std::vector<Move>& moves,
                     int from,
                     char side,
                     int file_delta,
                     int rank_delta) {
    int file = from % 8;
    int rank = from / 8;
    int next_file = file + file_delta;
    int next_rank = rank + rank_delta;
    while (next_file >= 0 && next_file < 8 && next_rank >= 0 && next_rank < 8) {
        int index = next_rank * 8 + next_file;
        char target = board.PieceAt(index);
        if (target == '.') {
            AddMove(moves, from, index);
        } else {
            if (IsEnemy(target, side)) {
                AddMove(moves, from, index);
            }
            break;
        }
        next_file += file_delta;
        next_rank += rank_delta;
    }
}

void AddKnightMoves(const Board& board, std::vector<Move>& moves, int from, char side) {
    const int file = from % 8;
    const int rank = from / 8;
    const int offsets[8][2] = {
        {1, 2},  {2, 1},  {-1, 2}, {-2, 1},
        {1, -2}, {2, -1}, {-1, -2}, {-2, -1},
    };
    for (const auto& offset : offsets) {
        int next_file = file + offset[0];
        int next_rank = rank + offset[1];
        if (next_file < 0 || next_file >= 8 || next_rank < 0 || next_rank >= 8) {
            continue;
        }
        int index = next_rank * 8 + next_file;
        char target = board.PieceAt(index);
        if (target == '.' || IsEnemy(target, side)) {
            AddMove(moves, from, index);
        }
    }
}

void AddKingMoves(const Board& board, std::vector<Move>& moves, int from, char side) {
    const int file = from % 8;
    const int rank = from / 8;
    for (int df = -1; df <= 1; ++df) {
        for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) {
                continue;
            }
            int next_file = file + df;
            int next_rank = rank + dr;
            if (next_file < 0 || next_file >= 8 || next_rank < 0 || next_rank >= 8) {
                continue;
            }
            int index = next_rank * 8 + next_file;
            char target = board.PieceAt(index);
            if (target == '.' || IsEnemy(target, side)) {
                AddMove(moves, from, index);
            }
        }
    }
}

void AddPawnMoves(const Board& board, std::vector<Move>& moves, int from, char side) {
    int file = from % 8;
    int rank = from / 8;
    if (side == 'w') {
        int forward = from + 8;
        if (rank < 7 && board.PieceAt(forward) == '.') {
            AddMove(moves, from, forward);
            if (rank == 1) {
                int double_forward = from + 16;
                if (board.PieceAt(double_forward) == '.') {
                    AddMove(moves, from, double_forward);
                }
            }
        }
        if (file > 0) {
            int capture_left = from + 7;
            if (rank < 7 && IsEnemy(board.PieceAt(capture_left), side)) {
                AddMove(moves, from, capture_left);
            }
        }
        if (file < 7) {
            int capture_right = from + 9;
            if (rank < 7 && IsEnemy(board.PieceAt(capture_right), side)) {
                AddMove(moves, from, capture_right);
            }
        }
    } else {
        int forward = from - 8;
        if (rank > 0 && board.PieceAt(forward) == '.') {
            AddMove(moves, from, forward);
            if (rank == 6) {
                int double_forward = from - 16;
                if (board.PieceAt(double_forward) == '.') {
                    AddMove(moves, from, double_forward);
                }
            }
        }
        if (file > 0) {
            int capture_left = from - 9;
            if (rank > 0 && IsEnemy(board.PieceAt(capture_left), side)) {
                AddMove(moves, from, capture_left);
            }
        }
        if (file < 7) {
            int capture_right = from - 7;
            if (rank > 0 && IsEnemy(board.PieceAt(capture_right), side)) {
                AddMove(moves, from, capture_right);
            }
        }
    }
}
}  // namespace

std::vector<Move> GeneratePseudoLegalMoves(const Board& board) {
    std::vector<Move> moves;
    char side = board.SideToMove();
    for (int index = 0; index < 64; ++index) {
        char piece = board.PieceAt(index);
        if (piece == '.' || !IsFriendly(piece, side)) {
            continue;
        }

        switch (piece) {
            case 'P':
            case 'p':
                AddPawnMoves(board, moves, index, side);
                break;
            case 'N':
            case 'n':
                AddKnightMoves(board, moves, index, side);
                break;
            case 'B':
            case 'b':
                AddSlidingMoves(board, moves, index, side, 1, 1);
                AddSlidingMoves(board, moves, index, side, -1, 1);
                AddSlidingMoves(board, moves, index, side, 1, -1);
                AddSlidingMoves(board, moves, index, side, -1, -1);
                break;
            case 'R':
            case 'r':
                AddSlidingMoves(board, moves, index, side, 1, 0);
                AddSlidingMoves(board, moves, index, side, -1, 0);
                AddSlidingMoves(board, moves, index, side, 0, 1);
                AddSlidingMoves(board, moves, index, side, 0, -1);
                break;
            case 'Q':
            case 'q':
                AddSlidingMoves(board, moves, index, side, 1, 0);
                AddSlidingMoves(board, moves, index, side, -1, 0);
                AddSlidingMoves(board, moves, index, side, 0, 1);
                AddSlidingMoves(board, moves, index, side, 0, -1);
                AddSlidingMoves(board, moves, index, side, 1, 1);
                AddSlidingMoves(board, moves, index, side, -1, 1);
                AddSlidingMoves(board, moves, index, side, 1, -1);
                AddSlidingMoves(board, moves, index, side, -1, -1);
                break;
            case 'K':
            case 'k':
                AddKingMoves(board, moves, index, side);
                break;
            default:
                break;
        }
    }
    return moves;
}
