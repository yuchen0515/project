#include <cstdlib>
#include <unistd.h>

#include <string>

#include "../arena/external_engine.h"
#include "../arena/match.h"
#include "../core/movegen.h"
#include "test.h"

using namespace pioneer;

namespace {

// The external engine is optional: these tests check the adapter when it
// is installed and check the failure path when it is not.
bool fairy_stockfish_available() {
    return system("command -v fairy-stockfish > /dev/null 2>&1") == 0;
}

}  // namespace

TEST(external_engine_survives_a_missing_binary) {
    ExternalEngineSpec spec;
    spec.label = "nonexistent";
    spec.command = "definitely-not-an-engine-binary";
    spec.depth = 1;

    ExternalEngine engine(spec);
    CHECK(!engine.error().empty());

    // Asking a dead engine for a move must return nothing rather than
    // hang or crash - the arena scores that as a forfeit.
    Game game;
    CHECK_EQ(engine.choose(game), MOVE_NONE);
}

TEST(external_engine_plays_legal_minishogi) {
    if (!fairy_stockfish_available()) {
        std::printf("skipping %s: fairy-stockfish not installed\n",
                    ::test::current());
        return;
    }

    ExternalEngine engine(fairy_stockfish_spec(4));
    CHECK(engine.running());
    CHECK(engine.error().empty());

    Game game;
    engine.new_game(1);

    // Play a handful of plies against it and check every reply is legal
    // in our rules - which also exercises the position/history encoding.
    for (int ply = 0; ply < 8 && !game.over(); ++ply) {
        MoveList legal;
        generate_legal(game.position(), legal);
        if (legal.empty()) break;

        const Move move = (game.side_to_move() == BLACK) ? legal[0]
                                                         : engine.choose(game);
        CHECK(move != MOVE_NONE);
        CHECK(legal.contains(move));
        CHECK(game.apply(move));
    }
}

TEST(external_engine_takes_a_mate_it_is_handed) {
    if (!fairy_stockfish_available()) return;

    // White to move and mate in one by dropping a gold on 5b; the black
    // king on 1e is boxed by its own pieces. If the adapter's position
    // encoding were wrong, this would come back as something else.
    ExternalEngine engine(fairy_stockfish_spec(6));
    Game game;
    CHECK(game.set("4K/5/3b1/5/2R1k w g 1"));
    engine.new_game(1);

    const Move move = engine.choose(game);
    CHECK(move != MOVE_NONE);

    MoveList legal;
    generate_legal(game.position(), legal);
    CHECK(legal.contains(move));
}
