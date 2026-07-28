// Transposition table.
//
// Minishogi transposes heavily - a five square board reaches the same
// position by many move orders, and the zobrist key that repetition
// detection already needs is exactly what indexes them. Without this the
// search re-derives the same subtree over and over.

#pragma once

#include <cstddef>
#include <vector>

#include "../core/types.h"
#include "eval.h"

namespace pioneer {

// Scores at or beyond this are mates, and have to be stored relative to
// the node rather than to the root.
constexpr int MATE_THRESHOLD = MATE_SCORE - 1000;

enum class Bound : uint8_t { NONE, LOWER, UPPER, EXACT };

struct TTEntry {
    uint64_t key = 0;
    Move move = MOVE_NONE;
    int16_t score = 0;
    int8_t depth = -1;
    Bound bound = Bound::NONE;
};

// A mate found at ply 6 is the same mate wherever the node sits in a
// later search, so the ply is folded out on the way in and back in on
// the way out.
constexpr int score_to_tt(int score, int ply) {
    if (score >= MATE_THRESHOLD) return score + ply;
    if (score <= -MATE_THRESHOLD) return score - ply;
    return score;
}
constexpr int score_from_tt(int score, int ply) {
    if (score >= MATE_THRESHOLD) return score - ply;
    if (score <= -MATE_THRESHOLD) return score + ply;
    return score;
}

class TranspositionTable {
   public:
    explicit TranspositionTable(size_t megabytes = 16);

    void clear();

    // Returns nullptr on a miss. The entry is only valid until the next
    // store() touches the same slot.
    const TTEntry* probe(uint64_t key) const;

    void store(uint64_t key, int depth, int score, Bound bound, Move move,
               int ply);

    size_t size() const { return entries_.size(); }

   private:
    TTEntry& slot(uint64_t key) { return entries_[key & mask_]; }
    const TTEntry& slot(uint64_t key) const { return entries_[key & mask_]; }

    std::vector<TTEntry> entries_;
    uint64_t mask_ = 0;
};

}  // namespace pioneer
