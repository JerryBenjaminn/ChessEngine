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

void AddMove(std::vector<Move>& moves, int from, int to, std::optional<char> promotion = std::nullopt) {
    moves.emplace_back(from, to, promotion);
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
            if (rank == 6) {
                AddMove(moves, from, forward, 'q');
                AddMove(moves, from, forward, 'r');
                AddMove(moves, from, forward, 'b');
                AddMove(moves, from, forward, 'n');
            } else {
                AddMove(moves, from, forward);
            }
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
                if (rank == 6) {
                    AddMove(moves, from, capture_left, 'q');
                    AddMove(moves, from, capture_left, 'r');
                    AddMove(moves, from, capture_left, 'b');
                    AddMove(moves, from, capture_left, 'n');
                } else {
                    AddMove(moves, from, capture_left);
                }
            }
        }
        if (file < 7) {
            int capture_right = from + 9;
            if (rank < 7 && IsEnemy(board.PieceAt(capture_right), side)) {
                if (rank == 6) {
                    AddMove(moves, from, capture_right, 'q');
                    AddMove(moves, from, capture_right, 'r');
                    AddMove(moves, from, capture_right, 'b');
                    AddMove(moves, from, capture_right, 'n');
                } else {
                    AddMove(moves, from, capture_right);
                }
            }
        }
    } else {
        int forward = from - 8;
        if (rank > 0 && board.PieceAt(forward) == '.') {
            if (rank == 1) {
                AddMove(moves, from, forward, 'q');
                AddMove(moves, from, forward, 'r');
                AddMove(moves, from, forward, 'b');
                AddMove(moves, from, forward, 'n');
            } else {
                AddMove(moves, from, forward);
            }
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
                if (rank == 1) {
                    AddMove(moves, from, capture_left, 'q');
                    AddMove(moves, from, capture_left, 'r');
                    AddMove(moves, from, capture_left, 'b');
                    AddMove(moves, from, capture_left, 'n');
                } else {
                    AddMove(moves, from, capture_left);
                }
            }
        }
        if (file < 7) {
            int capture_right = from - 7;
            if (rank > 0 && IsEnemy(board.PieceAt(capture_right), side)) {
                if (rank == 1) {
                    AddMove(moves, from, capture_right, 'q');
                    AddMove(moves, from, capture_right, 'r');
                    AddMove(moves, from, capture_right, 'b');
                    AddMove(moves, from, capture_right, 'n');
                } else {
                    AddMove(moves, from, capture_right);
                }
            }
        }
    }
}

Color ColorFromSide(char side) {
    return side == 'w' ? Color::White : Color::Black;
}

Color Opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

bool IsColorPiece(char piece, Color color) {
    return color == Color::White ? IsWhitePiece(piece) : IsBlackPiece(piece);
}

int FindKingSquare(const Board& board, Color color) {
    char king = color == Color::White ? 'K' : 'k';
    for (int i = 0; i < 64; ++i) {
        if (board.PieceAt(i) == king) {
            return i;
        }
    }
    return -1;
}

struct UndoMove {
    int from;
    int to;
    char moved;
    char captured;
    char side_to_move;
};

UndoMove ApplyMove(Board& board, const Move& move) {
    int from = move.from();
    int to = move.to();
    char moved = board.PieceAt(from);
    char captured = board.PieceAt(to);
    char side = board.SideToMove();
    if (move.promotion().has_value()) {
        char promo = move.promotion().value();
        char promoted_piece = side == 'w' ? static_cast<char>(promo - ('a' - 'A')) : promo;
        board.SetPieceAt(to, promoted_piece);
    } else {
        board.SetPieceAt(to, moved);
    }
    board.SetPieceAt(from, '.');
    return {from, to, moved, captured, side};
}

void UndoMoveApply(Board& board, const UndoMove& undo) {
    board.SetPieceAt(undo.from, undo.moved);
    board.SetPieceAt(undo.to, undo.captured);
    board.SetSideToMove(undo.side_to_move);
}
}  // namespace

bool IsSquareAttacked(const Board& board, int square, Color byColor) {
    if (square < 0 || square > 63) {
        return false;
    }

    int file = square % 8;
    int rank = square / 8;

    if (byColor == Color::White) {
        if (rank > 0 && file > 0 && board.PieceAt((rank - 1) * 8 + (file - 1)) == 'P') {
            return true;
        }
        if (rank > 0 && file < 7 && board.PieceAt((rank - 1) * 8 + (file + 1)) == 'P') {
            return true;
        }
    } else {
        if (rank < 7 && file > 0 && board.PieceAt((rank + 1) * 8 + (file - 1)) == 'p') {
            return true;
        }
        if (rank < 7 && file < 7 && board.PieceAt((rank + 1) * 8 + (file + 1)) == 'p') {
            return true;
        }
    }

    const int knight_offsets[8][2] = {
        {1, 2},  {2, 1},  {-1, 2}, {-2, 1},
        {1, -2}, {2, -1}, {-1, -2}, {-2, -1},
    };
    for (const auto& offset : knight_offsets) {
        int next_file = file + offset[0];
        int next_rank = rank + offset[1];
        if (next_file < 0 || next_file >= 8 || next_rank < 0 || next_rank >= 8) {
            continue;
        }
        char piece = board.PieceAt(next_rank * 8 + next_file);
        if (piece == (byColor == Color::White ? 'N' : 'n')) {
            return true;
        }
    }

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
            char piece = board.PieceAt(next_rank * 8 + next_file);
            if (piece == (byColor == Color::White ? 'K' : 'k')) {
                return true;
            }
        }
    }

    const int rook_dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto& dir : rook_dirs) {
        int next_file = file + dir[0];
        int next_rank = rank + dir[1];
        while (next_file >= 0 && next_file < 8 && next_rank >= 0 && next_rank < 8) {
            char piece = board.PieceAt(next_rank * 8 + next_file);
            if (piece != '.') {
                if (IsColorPiece(piece, byColor) && (piece == 'R' || piece == 'r' || piece == 'Q' || piece == 'q')) {
                    return true;
                }
                break;
            }
            next_file += dir[0];
            next_rank += dir[1];
        }
    }

    const int bishop_dirs[4][2] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
    for (const auto& dir : bishop_dirs) {
        int next_file = file + dir[0];
        int next_rank = rank + dir[1];
        while (next_file >= 0 && next_file < 8 && next_rank >= 0 && next_rank < 8) {
            char piece = board.PieceAt(next_rank * 8 + next_file);
            if (piece != '.') {
                if (IsColorPiece(piece, byColor) && (piece == 'B' || piece == 'b' || piece == 'Q' || piece == 'q')) {
                    return true;
                }
                break;
            }
            next_file += dir[0];
            next_rank += dir[1];
        }
    }

    return false;
}

bool InCheck(const Board& board, Color color) {
    int king_square = FindKingSquare(board, color);
    if (king_square == -1) {
        return false;
    }
    return IsSquareAttacked(board, king_square, Opposite(color));
}

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

std::vector<Move> GenerateLegalMoves(const Board& board) {
    std::vector<Move> legal;
    Color side = ColorFromSide(board.SideToMove());
    auto pseudo = GeneratePseudoLegalMoves(board);
    Board scratch = board;
    for (const auto& move : pseudo) {
        char target = scratch.PieceAt(move.to());
        if (target == (side == Color::White ? 'k' : 'K')) {
            continue;
        }
        UndoMove undo = ApplyMove(scratch, move);
        if (!InCheck(scratch, side)) {
            legal.push_back(move);
        }
        UndoMoveApply(scratch, undo);
    }
    return legal;
}

uint64_t Perft(const Board& board, int depth) {
    if (depth <= 0) {
        return 1;
    }

    uint64_t nodes = 0;
    auto moves = GenerateLegalMoves(board);
    Board scratch = board;
    for (const auto& move : moves) {
        UndoMove undo = ApplyMove(scratch, move);
        char next_side = undo.side_to_move == 'w' ? 'b' : 'w';
        scratch.SetSideToMove(next_side);
        nodes += Perft(scratch, depth - 1);
        UndoMoveApply(scratch, undo);
    }
    return nodes;
}
