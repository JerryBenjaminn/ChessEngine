#pragma once

#include <vector>

#include "Board.h"
#include "Move.h"

bool GetBookMove(const Board& board,
                 const std::vector<Move>& legalMoves,
                 int plyCount,
                 int maxBookPlies,
                 Move& outMove);
