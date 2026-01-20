#pragma once

#include <vector>

#include "Board.h"
#include "Move.h"

std::vector<Move> GeneratePseudoLegalMoves(const Board& board);
