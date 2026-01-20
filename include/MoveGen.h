#pragma once

#include <cstdint>
#include <vector>

#include "Board.h"
#include "Move.h"

enum class Color { White, Black };

std::vector<Move> GeneratePseudoLegalMoves(const Board& board);
std::vector<Move> GenerateLegalMoves(const Board& board);
uint64_t Perft(const Board& board, int depth);

bool IsSquareAttacked(const Board& board, int square, Color byColor);
bool InCheck(const Board& board, Color color);
