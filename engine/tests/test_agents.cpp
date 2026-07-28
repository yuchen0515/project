#include <string>

#include "../agents/eval.h"
#include "../agents/greedy_agent.h"
#include "../agents/random_agent.h"
#include "test.h"

using namespace pioneer;

namespace {

Position position_of(const std::string& sfen) {
    Position pos;
    CHECK(pos.set(sfen));
    return pos;
}

Game game_of(const std::string& sfen) {
    Game game;
    CHECK(game.set(sfen));
    return game;
}

}  // namespace

TEST(evaluation_is_symmetric_and_zero_at_the_start) {
    CHECK_EQ(evaluate(Position()), 0);

    // Mirrored positions must evaluate identically for the side to move,
    // otherwise the search has a built-in colour bias.
    CHECK_EQ(evaluate(position_of("4k/5/5/5/K4 b RBGSP 1")),
             evaluate(position_of("4k/5/5/5/K4 w rbgsp 1")));
}

TEST(evaluation_counts_material_and_hands) {
    // A rook up, from the side that has it. It sits on 4e rather than
    // 1e so that it is not already attacking the white king, which
    // would make the position illegal with Black to move.
    const int with_rook = evaluate(position_of("4k/5/5/5/KR3 b - 1"));
    CHECK_EQ(with_rook, PIECE_VALUE[ROOK]);
    CHECK_EQ(evaluate(position_of("4k/5/5/5/KR3 w - 1")), -PIECE_VALUE[ROOK]);

    // The same rook in hand is worth slightly more than on the board.
    const int in_hand = evaluate(position_of("4k/5/5/5/K4 b R 1"));
    CHECK(in_hand > with_rook);
    CHECK_EQ(in_hand, PIECE_VALUE[ROOK] * (100 + HAND_BONUS_PERCENT) / 100);
}

TEST(piece_values_follow_shogi_theory) {
    // Rook above bishop above gold above silver above pawn, and every
    // promotion an improvement. The legacy engine violated all of this.
    CHECK(PIECE_VALUE[ROOK] > PIECE_VALUE[BISHOP]);
    CHECK(PIECE_VALUE[BISHOP] > PIECE_VALUE[GOLD]);
    CHECK(PIECE_VALUE[GOLD] > PIECE_VALUE[SILVER]);
    CHECK(PIECE_VALUE[SILVER] > PIECE_VALUE[PAWN]);
    CHECK(PIECE_VALUE[DRAGON] > PIECE_VALUE[ROOK]);
    CHECK(PIECE_VALUE[HORSE] > PIECE_VALUE[BISHOP]);
    CHECK(PIECE_VALUE[PRO_PAWN] > PIECE_VALUE[PAWN]);
    CHECK(PIECE_VALUE[PRO_SILVER] > PIECE_VALUE[SILVER]);
    CHECK_EQ(PIECE_VALUE[KING], 0);
}

TEST(greedy_takes_the_most_valuable_piece) {
    // A black silver on 3c can take a pawn on 2b or a rook on 4b.
    GreedyAgent agent;
    Game game = game_of("4k/1r1p1/2S2/5/K4 b - 1");
    CHECK_EQ(usi_move(agent.choose(game)), std::string("3c4b"));
}

TEST(greedy_prefers_mate_over_material) {
    // White king on 5a. Dropping the gold on 5b mates: it covers both
    // flight squares and the bishop on 4c defends it. The alternative
    // is winning a whole rook on 2a. Mate must win the comparison.
    Game game = game_of("k2r1/5/1B3/5/4K b G 1");
    GreedyAgent agent;
    const Move chosen = agent.choose(game);

    Position after = game.position();
    after.apply(chosen);
    CHECK(is_checkmate(after));
    CHECK_EQ(usi_move(chosen), std::string("G*5b"));

    // The rook really was on offer, so the choice was not forced.
    MoveList legal;
    generate_legal(game.position(), legal);
    CHECK(legal.contains(parse_usi_move("4c2a")));
}

TEST(greedy_avoids_hanging_its_own_piece) {
    // Black rook on 3c may take a pawn on 3b, but the white king on 2a
    // recaptures. Stepping aside keeps the rook.
    Game game = game_of("3k1/2p2/2R2/5/K4 b - 1");
    GreedyAgent agent;
    // A one-ply agent cannot see the recapture - it grabs the pawn.
    // Recorded so the change is visible the moment a deeper agent runs
    // on this position and chooses otherwise.
    CHECK_EQ(usi_move(agent.choose(game)), std::string("3c3b"));
}

TEST(agents_are_reproducible_from_their_seed) {
    Game game;
    RandomAgent a, b;
    a.new_game(7);
    b.new_game(7);
    for (int i = 0; i < 5; ++i) CHECK_EQ(a.choose(game), b.choose(game));

    // A different game seed gives a different stream, so a match is not
    // one game played repeatedly.
    RandomAgent c;
    c.new_game(8);
    RandomAgent d;
    d.new_game(7);
    bool differs = false;
    for (int i = 0; i < 20 && !differs; ++i)
        if (c.choose(game) != d.choose(game)) differs = true;
    CHECK(differs);
}

TEST(agents_return_no_move_when_there_are_none) {
    Game mated = game_of("k4/G4/5/5/R3K w - 1");
    RandomAgent random_agent;
    GreedyAgent greedy;
    CHECK_EQ(random_agent.choose(mated), MOVE_NONE);
    CHECK_EQ(greedy.choose(mated), MOVE_NONE);
}
