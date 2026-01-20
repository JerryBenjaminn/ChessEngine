#pragma once

#include <chrono>

#include "Board.h"
#include "Move.h"

int EvaluateMaterial(const Board& board);
int SearchBestMove(Board& board, int depth, Move& outBestMove);
int SearchBestMoveTimed(Board& board,
                        int maxDepth,
                        std::chrono::steady_clock::time_point deadline,
                        Move& outBestMove,
                        int& outDepth,
                        uint64_t& outNodes,
                        uint64_t& outQNodes);
