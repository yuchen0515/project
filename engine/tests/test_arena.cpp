#include <cmath>
#include <string>

#include "../agents/factory.h"
#include "../agents/random_agent.h"
#include "../arena/match.h"
#include "../arena/rating.h"
#include "test.h"

using namespace pioneer;

namespace {

// Returns a move that is never legal, to check that the arena treats a
// broken agent as a forfeit instead of trusting it.
class BrokenAgent : public Agent {
   public:
    const char* name() const override { return "broken"; }
    Move choose(const Game&) override { return make_move(0, 0, true); }
};

bool near(double a, double b, double tolerance) {
    return std::fabs(a - b) <= tolerance;
}

}  // namespace

TEST(a_game_replays_exactly_from_its_seed) {
    RandomAgent black1, white1(99);
    RandomAgent black2, white2(99);
    const GameRecord first = play_game(black1, white1, 42);
    const GameRecord again = play_game(black2, white2, 42);

    CHECK_EQ(first.plies, again.plies);
    CHECK_EQ(first.final_sfen, again.final_sfen);
    CHECK_EQ(int(first.outcome), int(again.outcome));

    // A different seed gives a different game, otherwise a match would
    // be one game repeated.
    const GameRecord other = play_game(black2, white2, 43);
    CHECK(other.final_sfen != first.final_sfen);
}

TEST(games_end_for_a_recorded_reason) {
    RandomAgent black, white(7);
    const GameRecord record = play_game(black, white, 5);
    CHECK(record.plies > 0);
    CHECK(record.termination != Termination::ILLEGAL_MOVE);
    if (record.termination != Termination::PLY_LIMIT)
        CHECK(record.outcome != Outcome::ONGOING);
    else
        CHECK_EQ(int(record.outcome), int(Outcome::ONGOING));
}

TEST(ply_limit_leaves_the_game_unfinished_rather_than_inventing_a_result) {
    RandomAgent black, white(7);
    const GameRecord record = play_game(black, white, 5, 4);
    CHECK_EQ(record.plies, 4);
    CHECK_EQ(int(record.outcome), int(Outcome::ONGOING));
    CHECK_EQ(int(record.termination), int(Termination::PLY_LIMIT));
}

TEST(an_agent_returning_an_illegal_move_forfeits) {
    BrokenAgent broken;
    RandomAgent honest;

    const GameRecord as_black = play_game(broken, honest, 1);
    CHECK_EQ(int(as_black.termination), int(Termination::ILLEGAL_MOVE));
    CHECK_EQ(int(as_black.outcome), int(Outcome::WHITE_WIN));
    CHECK_EQ(as_black.plies, 0);

    const GameRecord as_white = play_game(honest, broken, 1);
    CHECK_EQ(int(as_white.termination), int(Termination::ILLEGAL_MOVE));
    CHECK_EQ(int(as_white.outcome), int(Outcome::BLACK_WIN));

    MatchResult result = play_match(broken, honest, 4);
    CHECK_EQ(result.illegal, 4);
    CHECK_EQ(result.wins_a, 0);
    CHECK_EQ(result.wins_b, 4);
}

TEST(match_alternates_colours_evenly) {
    RandomAgent a(1), b(2);
    const MatchResult result = play_match(a, b, 10);
    CHECK_EQ(result.games, 10);
    CHECK_EQ(result.a_as_black_games, 5);
    CHECK_EQ(result.wins_a + result.wins_b + result.unfinished, 10);
}

TEST(a_match_replays_from_its_base_seed) {
    RandomAgent a1(1), b1(2), a2(1), b2(2);
    const MatchResult first = play_match(a1, b1, 12, 500);
    const MatchResult again = play_match(a2, b2, 12, 500);
    CHECK_EQ(first.wins_a, again.wins_a);
    CHECK_EQ(first.wins_b, again.wins_b);
    CHECK_EQ(first.unfinished, again.unfinished);
}

TEST(greedy_beats_random_decisively) {
    // The ladder has to be ordered for any rating to mean anything. A
    // short match is enough: the gap is enormous, and the seed is fixed
    // so this cannot flake.
    auto greedy = make_agent("greedy", 1);
    auto random_agent = make_agent("random", 2);
    const MatchResult result = play_match(*greedy, *random_agent, 30, 7);
    CHECK(result.decisive() >= 25);
    CHECK(result.score_a() > 0.85);
    CHECK_EQ(result.illegal, 0);
}

TEST(elo_conversion_matches_the_logistic_curve) {
    CHECK(near(elo_difference(0.5), 0.0, 1e-9));
    CHECK(near(elo_difference(0.75), 190.85, 0.01));
    CHECK(near(elo_difference(0.25), -190.85, 0.01));
    // Symmetric about even, and monotonic.
    CHECK(near(elo_difference(0.6) + elo_difference(0.4), 0.0, 1e-9));
    CHECK(elo_difference(0.9) > elo_difference(0.8));
    CHECK(!std::isfinite(elo_difference(1.0)));
    CHECK(!std::isfinite(elo_difference(0.0)));
}

TEST(elo_interval_brackets_the_estimate_and_narrows_with_games) {
    const EloEstimate few = elo_estimate(0.6, 20);
    CHECK(few.finite);
    CHECK(few.lower < few.elo);
    CHECK(few.elo < few.upper);

    const EloEstimate many = elo_estimate(0.6, 2000);
    CHECK(many.upper - many.lower < few.upper - few.lower);

    // An even score at any sample size must not claim a difference.
    const EloEstimate even = elo_estimate(0.5, 100);
    CHECK(even.lower < 0.0 && even.upper > 0.0);

    // A clean sweep is unbounded above but still bounded below, which
    // is the only honest thing to report.
    const EloEstimate swept = elo_estimate(1.0, 30);
    CHECK(!swept.finite);
    CHECK(std::isfinite(swept.lower));
    CHECK(swept.lower > 0.0);
}
