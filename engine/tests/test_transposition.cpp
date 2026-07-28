#include "../agents/transposition.h"
#include "../agents/alphabeta_agent.h"
#include "../arena/match.h"
#include "test.h"

using namespace pioneer;

TEST(table_stores_and_returns_what_was_put_in) {
    TranspositionTable table(1);
    const uint64_t key = 0x1234'5678'9abc'def0ull;
    const Move move = parse_usi_move("5d5c");

    CHECK(table.probe(key) == nullptr);

    table.store(key, 4, 120, Bound::EXACT, move, 0);
    const TTEntry* entry = table.probe(key);
    CHECK(entry != nullptr);
    if (!entry) return;
    CHECK_EQ(entry->depth, 4);
    CHECK_EQ(int(entry->score), 120);
    CHECK_EQ(entry->move, move);
    CHECK(entry->bound == Bound::EXACT);

    // A different key must not collide into a hit.
    CHECK(table.probe(key ^ 1ull) == nullptr);
}

TEST(mate_scores_are_stored_relative_to_the_node) {
    // A mate in one found at ply 6 is a mate in one wherever the node
    // turns up later, so the ply is folded out on the way in.
    const int at_ply_six = MATE_SCORE - 7;
    const int stored = score_to_tt(at_ply_six, 6);
    CHECK_EQ(stored, MATE_SCORE - 1);
    CHECK_EQ(score_from_tt(stored, 6), at_ply_six);
    CHECK_EQ(score_from_tt(stored, 2), MATE_SCORE - 3);  // three plies away

    // Losses mirror.
    const int lost = -MATE_SCORE + 7;
    CHECK_EQ(score_from_tt(score_to_tt(lost, 6), 6), lost);

    // Ordinary scores pass through untouched.
    CHECK_EQ(score_to_tt(150, 9), 150);
    CHECK_EQ(score_from_tt(-150, 9), -150);
}

TEST(deeper_entries_are_not_evicted_by_shallower_ones) {
    TranspositionTable table(1);
    const uint64_t key = 42;

    table.store(key, 8, 100, Bound::EXACT, parse_usi_move("5d5c"), 0);
    table.store(key, 2, -100, Bound::UPPER, parse_usi_move("1e1d"), 0);

    const TTEntry* entry = table.probe(key);
    CHECK(entry != nullptr);
    if (entry) {
        CHECK_EQ(entry->depth, 8);
        CHECK_EQ(int(entry->score), 100);
    }

    // A different position may take the slot whatever its depth: the
    // table is a cache, not a database.
    table.store(key + table.size(), 1, 5, Bound::EXACT, MOVE_NONE, 0);
    CHECK(table.probe(key) == nullptr);
}

TEST(table_is_cleared_between_games) {
    // Entries carried across games would make a result depend on the
    // games played before it, and the arena promises reproducibility.
    TranspositionTable table(1);
    table.store(7, 3, 50, Bound::EXACT, MOVE_NONE, 0);
    CHECK(table.probe(7) != nullptr);
    table.clear();
    CHECK(table.probe(7) == nullptr);
}

TEST(transpositions_do_not_cost_playing_strength) {
    // A transposition table changes how long a fixed-depth search takes,
    // not what it concludes. Anything worse than level here means the
    // bounds or the mate-score adjustment are wrong.
    AlphaBetaAgent::Options with_table;
    with_table.depth = 3;
    AlphaBetaAgent::Options without = with_table;
    without.transposition = false;

    AlphaBetaAgent a(with_table, 1), b(without, 2);
    const MatchResult result = play_match(a, b, 30, 5);
    CHECK_EQ(result.illegal, 0);
    CHECK(result.score_a() > 0.35);
}
