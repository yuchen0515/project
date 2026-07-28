#include "transposition.h"

namespace pioneer {

TranspositionTable::TranspositionTable(size_t megabytes) {
    size_t entries = (megabytes * 1024 * 1024) / sizeof(TTEntry);

    // Round down to a power of two so indexing is a mask rather than a
    // division, and keep at least a token table.
    size_t power = 1;
    while (power * 2 <= entries) power *= 2;
    if (power < 1024) power = 1024;

    entries_.assign(power, TTEntry{});
    mask_ = power - 1;
}

void TranspositionTable::clear() {
    entries_.assign(entries_.size(), TTEntry{});
}

const TTEntry* TranspositionTable::probe(uint64_t key) const {
    const TTEntry& entry = slot(key);
    return (entry.bound != Bound::NONE && entry.key == key) ? &entry : nullptr;
}

void TranspositionTable::store(uint64_t key, int depth, int score, Bound bound,
                               Move move, int ply) {
    TTEntry& entry = slot(key);

    // Depth-preferred: a shallow result should not evict a deep one for
    // the same position, but anything may evict a different position.
    if (entry.bound != Bound::NONE && entry.key == key && entry.depth > depth)
        return;

    entry.key = key;
    entry.move = move;
    entry.score = int16_t(score_to_tt(score, ply));
    entry.depth = int8_t(depth);
    entry.bound = bound;
}

}  // namespace pioneer
