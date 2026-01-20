#include "MoveGen.h"

#ifdef CHESSENGINE_DEBUG
#include <cstdlib>
#include <iostream>
#endif

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
    int ep_square = board.EnPassantSquare();
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
        if (rank == 4) {
            if (file > 0 && ep_square == from + 7 && board.PieceAt(from - 1) == 'p') {
                AddMove(moves, from, from + 7);
            }
            if (file < 7 && ep_square == from + 9 && board.PieceAt(from + 1) == 'p') {
                AddMove(moves, from, from + 9);
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
        if (rank == 3) {
            if (file > 0 && ep_square == from - 9 && board.PieceAt(from - 1) == 'P') {
                AddMove(moves, from, from - 9);
            }
            if (file < 7 && ep_square == from - 7 && board.PieceAt(from + 1) == 'P') {
                AddMove(moves, from, from - 7);
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

void RemoveCastlingRight(std::string& rights, char right) {
    if (rights == "-") {
        rights.clear();
    }
    auto pos = rights.find(right);
    if (pos != std::string::npos) {
        rights.erase(pos, 1);
    }
    if (rights.empty()) {
        rights = "-";
    }
}

}  // namespace

MoveUndo ApplyMove(Board& board, const Move& move) {
#ifdef CHESSENGINE_DEBUG
    char moving = board.PieceAt(move.from());
    if (moving == '.' || moving == '\0') {
        std::cerr << "DEBUG ASSERT: move from empty square\n";
        std::abort();
    }
    if (board.SideToMove() == 'w' && !(moving >= 'A' && moving <= 'Z')) {
        std::cerr << "DEBUG ASSERT: wrong side to move (white expected)\n";
        std::abort();
    }
    if (board.SideToMove() == 'b' && !(moving >= 'a' && moving <= 'z')) {
        std::cerr << "DEBUG ASSERT: wrong side to move (black expected)\n";
        std::abort();
    }
#endif
    int from = move.from();
    int to = move.to();
    char moved = board.PieceAt(from);
    char captured = board.PieceAt(to);
    char side = board.SideToMove();
    int prev_ep = board.EnPassantSquare();
    std::string prev_castling = board.CastlingRights();
    int prev_halfmove = board.HalfmoveClock();
    int ep_capture_square = -1;
    char ep_captured = '.';
    bool was_en_passant = false;
    int rook_from = -1;
    int rook_to = -1;
    char rook_piece = '.';
    bool was_castling = false;

    board.SetEnPassantSquare(-1);

    if ((moved == 'P' || moved == 'p') && to == prev_ep && captured == '.') {
        was_en_passant = true;
        ep_capture_square = side == 'w' ? to - 8 : to + 8;
        ep_captured = board.PieceAt(ep_capture_square);
        board.SetPieceAt(ep_capture_square, '.');
    }

    char placed = moved;
    if (move.promotion().has_value()) {
        char promo = move.promotion().value();
        placed = side == 'w' ? static_cast<char>(promo - ('a' - 'A')) : promo;
    }

    board.SetPieceAt(to, placed);
    board.SetPieceAt(from, '.');

    if ((moved == 'K' && from == 4) || (moved == 'k' && from == 60)) {
        std::string rights = board.CastlingRights();
        if (moved == 'K') {
            RemoveCastlingRight(rights, 'K');
            RemoveCastlingRight(rights, 'Q');
        } else {
            RemoveCastlingRight(rights, 'k');
            RemoveCastlingRight(rights, 'q');
        }
        board.SetCastlingRights(rights);
    }

    if (moved == 'R') {
        std::string rights = board.CastlingRights();
        if (from == 0) {
            RemoveCastlingRight(rights, 'Q');
        } else if (from == 7) {
            RemoveCastlingRight(rights, 'K');
        }
        board.SetCastlingRights(rights);
    } else if (moved == 'r') {
        std::string rights = board.CastlingRights();
        if (from == 56) {
            RemoveCastlingRight(rights, 'q');
        } else if (from == 63) {
            RemoveCastlingRight(rights, 'k');
        }
        board.SetCastlingRights(rights);
    }

    if (captured == 'R') {
        std::string rights = board.CastlingRights();
        if (to == 0) {
            RemoveCastlingRight(rights, 'Q');
        } else if (to == 7) {
            RemoveCastlingRight(rights, 'K');
        }
        board.SetCastlingRights(rights);
    } else if (captured == 'r') {
        std::string rights = board.CastlingRights();
        if (to == 56) {
            RemoveCastlingRight(rights, 'q');
        } else if (to == 63) {
            RemoveCastlingRight(rights, 'k');
        }
        board.SetCastlingRights(rights);
    }

    if ((moved == 'K' || moved == 'k') && (to - from == 2 || from - to == 2)) {
        was_castling = true;
        if (moved == 'K') {
            if (to == 6) {
                rook_from = 7;
                rook_to = 5;
            } else if (to == 2) {
                rook_from = 0;
                rook_to = 3;
            }
        } else {
            if (to == 62) {
                rook_from = 63;
                rook_to = 61;
            } else if (to == 58) {
                rook_from = 56;
                rook_to = 59;
            }
        }
        if (rook_from >= 0 && rook_to >= 0) {
            rook_piece = board.PieceAt(rook_from);
            board.SetPieceAt(rook_to, rook_piece);
            board.SetPieceAt(rook_from, '.');
        }
    }

    if (moved == 'P' && to - from == 16) {
        board.SetEnPassantSquare(from + 8);
    } else if (moved == 'p' && from - to == 16) {
        board.SetEnPassantSquare(from - 8);
    }

    if (moved == 'P' || moved == 'p' || captured != '.' || was_en_passant) {
        board.SetHalfmoveClock(0);
    } else {
        board.SetHalfmoveClock(prev_halfmove + 1);
    }

#ifdef CHESSENGINE_DEBUG
    uint64_t before = board.Hash();
    board.RecomputeHash();
    if (board.Hash() != before) {
        std::cerr << "DEBUG ASSERT: hash mismatch after apply\n";
        std::abort();
    }
    board.DebugAssertValid();
#endif

    return {from,
            to,
            moved,
            captured,
            side,
            prev_ep,
            ep_capture_square,
            ep_captured,
            was_en_passant,
            rook_from,
            rook_to,
            rook_piece,
            was_castling,
            prev_castling,
            prev_halfmove};
}

void UndoMoveApply(Board& board, const MoveUndo& undo) {
    board.SetPieceAt(undo.from, undo.moved);
    board.SetPieceAt(undo.to, undo.captured);
    if (undo.was_en_passant && undo.ep_capture_square >= 0) {
        board.SetPieceAt(undo.ep_capture_square, undo.ep_captured);
    }
    if (undo.was_castling && undo.rook_from >= 0 && undo.rook_to >= 0) {
        board.SetPieceAt(undo.rook_from, undo.rook_piece);
        board.SetPieceAt(undo.rook_to, '.');
    }
    board.SetSideToMove(undo.side_to_move);
    board.SetEnPassantSquare(undo.prev_en_passant);
    board.SetCastlingRights(undo.prev_castling_rights);
    board.SetHalfmoveClock(undo.prev_halfmove_clock);
#ifdef CHESSENGINE_DEBUG
    uint64_t before = board.Hash();
    board.RecomputeHash();
    if (board.Hash() != before) {
        std::cerr << "DEBUG ASSERT: hash mismatch after undo\n";
        std::abort();
    }
    board.DebugAssertValid();
#endif
}

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

void AddCastlingMoves(const Board& board, std::vector<Move>& moves, char side) {
    Color color = ColorFromSide(side);
    if (InCheck(board, color)) {
        return;
    }

    const std::string& rights = board.CastlingRights();
    if (side == 'w') {
        if (rights.find('K') != std::string::npos) {
            if (board.PieceAt(5) == '.' && board.PieceAt(6) == '.' &&
                !IsSquareAttacked(board, 5, Color::Black) &&
                !IsSquareAttacked(board, 6, Color::Black)) {
                AddMove(moves, 4, 6);
            }
        }
        if (rights.find('Q') != std::string::npos) {
            if (board.PieceAt(1) == '.' && board.PieceAt(2) == '.' && board.PieceAt(3) == '.' &&
                !IsSquareAttacked(board, 3, Color::Black) &&
                !IsSquareAttacked(board, 2, Color::Black)) {
                AddMove(moves, 4, 2);
            }
        }
    } else {
        if (rights.find('k') != std::string::npos) {
            if (board.PieceAt(61) == '.' && board.PieceAt(62) == '.' &&
                !IsSquareAttacked(board, 61, Color::White) &&
                !IsSquareAttacked(board, 62, Color::White)) {
                AddMove(moves, 60, 62);
            }
        }
        if (rights.find('q') != std::string::npos) {
            if (board.PieceAt(57) == '.' && board.PieceAt(58) == '.' && board.PieceAt(59) == '.' &&
                !IsSquareAttacked(board, 59, Color::White) &&
                !IsSquareAttacked(board, 58, Color::White)) {
                AddMove(moves, 60, 58);
            }
        }
    }
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
                AddCastlingMoves(board, moves, side);
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
        MoveUndo undo = ApplyMove(scratch, move);
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
        MoveUndo undo = ApplyMove(scratch, move);
        char next_side = undo.side_to_move == 'w' ? 'b' : 'w';
        scratch.SetSideToMove(next_side);
        nodes += Perft(scratch, depth - 1);
        UndoMoveApply(scratch, undo);
    }
    return nodes;
}
