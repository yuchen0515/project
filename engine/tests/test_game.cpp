#include <string>

#include "../core/game.h"
#include "test.h"

using namespace pioneer;

namespace {

int outcome_code(Outcome o) { return static_cast<int>(o); }

}  // namespace

TEST(game_rejects_illegal_moves) {
    Game game;
    CHECK(!game.apply_usi("5d5b"));   // pawns move one square
    CHECK(!game.apply_usi("1a1b"));   // not White's turn
    CHECK(!game.apply(MOVE_NONE));
    CHECK_EQ(game.ply(), 0);

    CHECK(game.apply_usi("5d5c"));
    CHECK_EQ(game.ply(), 1);
    CHECK_EQ(int(game.side_to_move()), int(WHITE));
}

TEST(checkmate_ends_the_game) {
    Game game("k4/G4/5/5/R3K w - 1");
    CHECK(game.over());
    CHECK_EQ(outcome_code(game.outcome()), outcome_code(Outcome::BLACK_WIN));
    CHECK_EQ(std::string(game.termination()), "checkmate");
    CHECK(!game.apply_usi("5a4a"));  // nothing is playable once it is over
}

TEST(stalemate_is_a_loss_not_a_draw) {
    Game game("k4/4R/5/5/1R2K w - 1");
    CHECK(!game.position().in_check());
    CHECK(game.over());
    CHECK_EQ(outcome_code(game.outcome()), outcome_code(Outcome::BLACK_WIN));
    CHECK_EQ(std::string(game.termination()), "stalemate");
}

TEST(fourth_repetition_loses_for_the_first_player) {
    // Both kings shuffle between two squares. Each four-move cycle
    // reproduces the starting position with Black to move.
    Game game("4k/5/5/5/K4 b - 1");
    CHECK_EQ(game.repetition_count(), 1);

    const char* cycle[] = {"5e4e", "1a2a", "4e5e", "2a1a"};
    for (int lap = 2; lap <= 4; ++lap) {
        for (const char* m : cycle) CHECK(game.apply_usi(m));
        CHECK_EQ(game.repetition_count(), lap);
        CHECK_EQ(game.over(), lap >= REPETITION_LIMIT);
    }

    CHECK_EQ(outcome_code(game.outcome()), outcome_code(Outcome::WHITE_WIN));
    CHECK_EQ(std::string(game.termination()), "repetition");
    CHECK_EQ(game.ply(), 12);
}

TEST(repetition_counts_positions_not_moves) {
    // Reaching the same position by a different route still counts.
    Game game("4k/5/5/5/K4 b - 1");
    for (const char* m : {"5e4e", "1a2a", "4e4d", "2a2b", "4d4e", "2b2a"})
        CHECK(game.apply_usi(m));
    // Black king back on 4e, white king back on 2a, Black to move -
    // the position after White's reply, now seen a second time even
    // though the kings took a detour to get back to it.
    CHECK_EQ(game.repetition_count(), 2);

    for (const char* m : {"4e5e", "2a1a"}) CHECK(game.apply_usi(m));
    CHECK_EQ(game.repetition_count(), 2);  // back to the start position
}

TEST(hands_and_side_to_move_are_part_of_position_identity) {
    // Two positions with identical boards but different hands must not
    // count as a repetition.
    Game game("4k/5/5/5/K4 b P 1");
    CHECK(game.apply_usi("P*3c"));
    CHECK_EQ(game.repetition_count(), 1);
    CHECK(game.apply_usi("1a1b"));
    CHECK_EQ(game.repetition_count(), 1);
}
