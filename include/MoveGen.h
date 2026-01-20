#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Board.h"
#include "Move.h"

enum class Color { White, Black };

struct MoveUndo {
    int from;
    int to;
    char moved;
    char captured;
    char side_to_move;
    int prev_en_passant;
    int ep_capture_square;
    char ep_captured;
    bool was_en_passant;
    int rook_from;
    int rook_to;
    char rook_piece;
    bool was_castling;
    std::string prev_castling_rights;
};

std::vector<Move> GeneratePseudoLegalMoves(const Board& board);
std::vector<Move> GenerateLegalMoves(const Board& board);
uint64_t Perft(const Board& board, int depth);

MoveUndo ApplyMove(Board& board, const Move& move);
void UndoMoveApply(Board& board, const MoveUndo& undo);

bool IsSquareAttacked(const Board& board, int square, Color byColor);
bool InCheck(const Board& board, Color color);
