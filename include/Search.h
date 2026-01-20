#pragma once

#include "Board.h"
#include "Move.h"

int EvaluateMaterial(const Board& board);
int SearchBestMove(Board& board, int depth, Move& outBestMove);
