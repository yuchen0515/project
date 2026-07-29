#include <memory>
#include <sstream>
#include <string>

#include "../agents/random_agent.h"
#include "../usi/usi.h"
#include "test.h"

using namespace pioneer;

namespace {

// Feeds commands to a fresh engine and returns everything it printed.
struct Session {
    UsiEngine engine{std::make_unique<RandomAgent>()};
    std::ostringstream out;

    std::string send(const std::string& line) {
        const size_t before = out.str().size();
        engine.handle(line, out);
        return out.str().substr(before);
    }
};

class MovetimeSpyAgent : public Agent {
   public:
    const char* name() const override { return "movetime-spy"; }

    Move choose(const Game& game) override {
        last_movetime_ms = 0;
        MoveList legal;
        generate_legal(game.position(), legal);
        return legal.empty() ? MOVE_NONE : legal[0];
    }

    Move choose(const Game& game, int movetime_ms) override {
        last_movetime_ms = movetime_ms;
        MoveList legal;
        generate_legal(game.position(), legal);
        return legal.empty() ? MOVE_NONE : legal[0];
    }

    int last_movetime_ms = -1;
};

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

}  // namespace

TEST(usi_handshake) {
    Session s;
    const std::string reply = s.send("usi");
    CHECK(contains(reply, "id name Pioneer"));
    CHECK(contains(reply, "id author"));
    CHECK(contains(reply, "usiok"));
    CHECK_EQ(s.send("isready"), std::string("readyok\n"));
}

TEST(usi_quit_and_unknown_commands) {
    Session s;
    std::ostringstream sink;
    CHECK(s.engine.handle("isready", sink));
    CHECK(!s.engine.handle("quit", sink));
    CHECK(contains(s.send("frobnicate"), "unknown command"));
    CHECK_EQ(s.send(""), std::string(""));  // blank lines are ignored
}

TEST(usi_position_startpos_with_moves) {
    Session s;
    s.send("position startpos moves 5d5c 1b1c");
    CHECK_EQ(s.send("sfen"),
             std::string("rbsgk/5/P3p/5/KGSBR b - 3\n"));
    CHECK_EQ(s.engine.game().ply(), 2);
}

TEST(usi_position_sfen_with_and_without_moves) {
    Session s;
    s.send("position sfen 4k/5/5/5/K4 b P 1");
    CHECK_EQ(s.send("sfen"), std::string("4k/5/5/5/K4 b P 1\n"));

    s.send("position sfen 4k/5/5/5/K4 b P 1 moves P*3c");
    CHECK_EQ(s.send("sfen"), std::string("4k/5/2P2/5/K4 w - 2\n"));
}

TEST(usi_rejects_bad_input_without_dying) {
    Session s;
    CHECK(contains(s.send("position sfen 9z/nonsense b - 1"), "illegal position"));
    CHECK(contains(s.send("position startpos moves 5d5b"), "illegal move: 5d5b"));
    CHECK(contains(s.send("position wherever"), "unknown position type"));

    // Still functional afterwards.
    s.send("position startpos");
    CHECK_EQ(s.send("sfen"), std::string(START_SFEN) + "\n");
}

TEST(usi_go_returns_a_legal_move) {
    Session s;
    s.send("position startpos");
    const std::string reply = s.send("go btime 1000 wtime 1000");
    CHECK(contains(reply, "bestmove "));

    const Move played = parse_usi_move(reply.substr(reply.find(' ') + 1));
    MoveList legal;
    generate_legal(s.engine.game().position(), legal);
    CHECK(legal.contains(played));
}

TEST(usi_go_movetime_is_a_one_call_override) {
    auto spy = std::make_unique<MovetimeSpyAgent>();
    MovetimeSpyAgent* observed = spy.get();
    UsiEngine engine(std::move(spy));
    std::ostringstream out;

    engine.handle("position startpos", out);
    engine.handle("go movetime 37", out);
    CHECK_EQ(observed->last_movetime_ms, 37);

    // A later bare go must not inherit the previous command's budget.
    engine.handle("go", out);
    CHECK_EQ(observed->last_movetime_ms, 0);
}

TEST(usi_go_resigns_when_the_game_is_over) {
    Session s;
    s.send("position sfen k4/G4/5/5/R3K w - 1");  // white is mated
    CHECK_EQ(s.send("go"), std::string("bestmove resign\n"));
}

TEST(usi_go_perft_matches_the_engine) {
    Session s;
    s.send("position startpos");
    CHECK_EQ(s.send("go perft 4"), std::string("35401\n"));
}

TEST(usi_moves_command_lists_legal_moves) {
    Session s;
    s.send("position sfen r3k/5/5/5/K4 b - 1");
    const std::string reply = s.send("moves");
    CHECK(contains(reply, "5e4d"));
    CHECK(contains(reply, "5e4e"));
    CHECK(!contains(reply, "5e5d"));  // stays on the checking rook's file
}

TEST(usinewgame_resets_position_and_agent) {
    Session s;
    s.send("position startpos moves 5d5c");
    s.send("usinewgame");
    CHECK_EQ(s.send("sfen"), std::string(START_SFEN) + "\n");
    CHECK_EQ(s.engine.game().ply(), 0);
}

TEST(random_agent_is_reproducible_across_games) {
    // Same seed, same reset point, same sequence - a match can be
    // replayed exactly from its seed.
    RandomAgent a, b;
    Game game;
    Move first[4], second[4];
    for (int i = 0; i < 4; ++i) first[i] = a.choose(game);
    b.new_game(0);
    for (int i = 0; i < 4; ++i) second[i] = b.choose(game);
    for (int i = 0; i < 4; ++i) CHECK_EQ(first[i], second[i]);

    // Different seeds diverge, otherwise self-play would be one game
    // repeated forever.
    RandomAgent other(12345);
    bool differs = false;
    for (int i = 0; i < 8 && !differs; ++i)
        if (other.choose(game) != first[0]) differs = true;
    CHECK(differs);
}
