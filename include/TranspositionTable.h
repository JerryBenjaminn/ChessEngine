#pragma once

#include <cstdint>
#include <vector>

#include "Move.h"

enum class Bound { EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t key = 0;
    int depth = -1;
    int score = 0;
    Bound bound = Bound::EXACT;
    Move bestMove = Move(0, 0);
    bool hasBestMove = false;
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t size_power_of_two);

    void Clear();
    bool Probe(uint64_t key,
               int depth,
               int alpha,
               int beta,
               int& outScore,
               Move& outBestMove) const;
    void Store(uint64_t key, int depth, int score, Bound bound, const Move* bestMove);
    bool PeekBestMove(uint64_t key, Move& outBestMove) const;

private:
    size_t mask_;
    std::vector<TTEntry> entries_;
};
