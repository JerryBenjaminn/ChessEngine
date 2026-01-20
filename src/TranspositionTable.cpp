#include "TranspositionTable.h"

TranspositionTable::TranspositionTable(size_t size_power_of_two)
    : mask_(size_power_of_two - 1), entries_(size_power_of_two) {}

void TranspositionTable::Clear() {
    for (auto& entry : entries_) {
        entry = TTEntry{};
    }
}

bool TranspositionTable::Probe(uint64_t key,
                               int depth,
                               int alpha,
                               int beta,
                               int& outScore,
                               Move& outBestMove) const {
    const TTEntry& entry = entries_[key & mask_];
    if (entry.depth < 0 || entry.key != key || entry.depth < depth) {
        return false;
    }

    if (entry.bound == Bound::EXACT) {
        outScore = entry.score;
    } else if (entry.bound == Bound::LOWER) {
        if (entry.score < beta) {
            return false;
        }
        outScore = entry.score;
    } else {
        if (entry.score > alpha) {
            return false;
        }
        outScore = entry.score;
    }

    if (entry.hasBestMove) {
        outBestMove = entry.bestMove;
    }
    return true;
}

void TranspositionTable::Store(uint64_t key, int depth, int score, Bound bound, const Move* bestMove) {
    TTEntry& entry = entries_[key & mask_];
    if (entry.depth >= depth && entry.key == key) {
        return;
    }

    entry.key = key;
    entry.depth = depth;
    entry.score = score;
    entry.bound = bound;
    if (bestMove != nullptr) {
        entry.bestMove = *bestMove;
        entry.hasBestMove = true;
    } else {
        entry.hasBestMove = false;
    }
}

bool TranspositionTable::PeekBestMove(uint64_t key, Move& outBestMove) const {
    const TTEntry& entry = entries_[key & mask_];
    if (entry.depth < 0 || entry.key != key || !entry.hasBestMove) {
        return false;
    }
    outBestMove = entry.bestMove;
    return true;
}
