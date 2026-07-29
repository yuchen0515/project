#include <chrono>
#include <string>

#include "../agents/alphabeta_agent.h"
#include "../agents/eval.h"
#include "../agents/factory.h"
#include "../agents/greedy_agent.h"
#include "../agents/mcts_agent.h"
#include "../agents/random_agent.h"
#include "../arena/match.h"
#include "alphabeta_test_access.h"
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

    // The two sides sit 180 degrees apart, so the mirror of a position
    // flips rank and file and swaps colours. It must evaluate the same
    // for whoever is to move, or the engine has a built-in colour bias
    // that no amount of search can correct.
    CHECK_EQ(evaluate(position_of("2gk1/4p/5/P4/KG3 b S 1")),
             evaluate(position_of("3gk/4p/5/P4/1KG2 w s 1")));

    // A position that is its own mirror must score the same whoever is
    // to move, which catches a term that forgets to flip.
    CHECK_EQ(evaluate(position_of("1r2k/5/5/5/K2R1 b - 1")),
             evaluate(position_of("1r2k/5/5/5/K2R1 w - 1")));
}

TEST(evaluation_counts_material_and_hands) {
    // A rook up, from the side that has it. It sits on 4e rather than
    // 1e so that it is not already attacking the white king, which
    // would make the position illegal with Black to move.
    // Material only, so the placement and safety terms do not muddy
    // what this test is about.
    constexpr EvalWeights only_material{100, 0, 0};

    const int with_rook = evaluate(position_of("4k/5/5/5/KR3 b - 1"), only_material);
    CHECK_EQ(with_rook, PIECE_VALUE[ROOK]);
    CHECK_EQ(evaluate(position_of("4k/5/5/5/KR3 w - 1"), only_material),
             -PIECE_VALUE[ROOK]);

    // The same rook in hand is worth slightly more than on the board.
    const int in_hand =
        evaluate(position_of("4k/5/5/5/K4 b R 1"), only_material);
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

TEST(alphabeta_finds_mate_in_one) {
    Game game = game_of("k2r1/5/1B3/5/4K b G 1");
    for (int depth = 1; depth <= 4; ++depth) {
        AlphaBetaAgent agent(depth);
        CHECK_EQ(usi_move(agent.choose(game)), std::string("G*5b"));
    }
}

TEST(alphabeta_declines_the_poisoned_pawn) {
    // The pawn on 3b is defended by the king on 2a. Greedy takes it and
    // loses the rook; a search that sees one reply does not. Any safe
    // rook move will do, so the assertion is on what it avoids.
    Game game = game_of("3k1/2p2/2R2/5/K4 b - 1");
    CHECK_EQ(usi_move(GreedyAgent().choose(game)), std::string("3c3b"));

    for (int depth = 2; depth <= 4; ++depth) {
        AlphaBetaAgent agent(depth);
        const Move chosen = agent.choose(game);
        CHECK(usi_move(chosen) != std::string("3c3b"));

        MoveList legal;
        generate_legal(game.position(), legal);
        CHECK(legal.contains(chosen));
    }
}

TEST(alphabeta_prolongs_a_lost_position_and_hurries_a_won_one) {
    // Mate scores carry the ply, so a forced mate is taken at once
    // rather than after an idle shuffle.
    Game game = game_of("k2r1/5/1B3/5/4K b G 1");
    AlphaBetaAgent agent(4);
    Position after = game.position();
    after.apply(agent.choose(game));
    CHECK(is_checkmate(after));
}

TEST(quiescence_beats_the_same_depth_without_it) {
    // Evaluating in the middle of an exchange is what quiescence is
    // there to prevent, and at equal depth the difference is large.
    AlphaBetaAgent with_qs(2, 1, true), without_qs(2, 2, false);
    const MatchResult result = play_match(with_qs, without_qs, 20, 3);
    CHECK_EQ(result.illegal, 0);
    CHECK(result.score_a() > 0.65);
}

TEST(deeper_search_beats_shallower) {
    // The one result that shows a search is really searching. Depths of
    // the same parity, for the reason the next test documents.
    AlphaBetaAgent deep(4, 1), shallow(2, 2);
    const MatchResult result = play_match(deep, shallow, 20, 11);
    CHECK_EQ(result.illegal, 0);
    CHECK(result.score_a() > 0.7);
}

TEST(positional_terms_stay_off_until_they_are_tuned) {
    // They win self-play and lose to an outside engine, so they are
    // opt-in. This pins that down: a later change that quietly enables
    // them should have to argue with a failing test.
    CHECK_EQ(DEFAULT_WEIGHTS.placement, 0);
    CHECK_EQ(DEFAULT_WEIGHTS.king_safety, 0);
    CHECK_EQ(std::string(AlphaBetaAgent(4, 1).name()), "alphabeta-d4");

    AlphaBetaAgent::Options tuned;
    tuned.weights.placement = 100;
    tuned.weights.king_safety = 100;
    CHECK_EQ(std::string(AlphaBetaAgent(tuned, 1).name()), "alphabeta-d4+pst+ks");
}

TEST(move_ordering_state_resets_for_every_new_game) {
    AlphaBetaAgent agent(4, 1);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_killers(agent), 0);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_history(agent), 0);

    agent.new_game(7);
    CHECK(agent.choose(Game()) != MOVE_NONE);
    CHECK(AlphaBetaAgentTestAccess::nonzero_killers(agent) > 0);
    CHECK(AlphaBetaAgentTestAccess::nonzero_history(agent) > 0);

    agent.new_game(8);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_killers(agent), 0);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_history(agent), 0);
}

TEST(noorder_disables_killer_and_history_state) {
    AlphaBetaAgent::Options options;
    options.depth = 4;
    options.move_ordering = false;
    AlphaBetaAgent noorder(options, 1);
    CHECK_EQ(std::string(noorder.name()), "alphabeta-d4-noorder");

    noorder.new_game(7);
    CHECK(noorder.choose(Game()) != MOVE_NONE);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_killers(noorder), 0);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_history(noorder), 0);

    auto from_factory = make_agent("alphabeta-d3-noorder", 2);
    CHECK(from_factory != nullptr);
    CHECK_EQ(std::string(from_factory->name()), "alphabeta-d3-noorder");
}

TEST(quiescence_does_not_update_killer_or_history_state) {
    AlphaBetaAgent depth_one(1, 1);
    depth_one.new_game(7);
    CHECK(depth_one.choose(Game()) != MOVE_NONE);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_killers(depth_one), 0);
    CHECK_EQ(AlphaBetaAgentTestAccess::nonzero_history(depth_one), 0);
}

TEST(time_mode_name_and_factory_precedence) {
    AlphaBetaAgent::Options timed;
    timed.depth = 2;
    timed.movetime_ms = 17;
    CHECK_EQ(std::string(AlphaBetaAgent(timed, 1).name()),
             "alphabeta-t17");

    auto depth_then_time =
        make_agent("alphabeta-d2-t17-noorder", 2);
    auto time_then_depth = make_agent("alphabeta-t17-d2", 3);
    CHECK(depth_then_time != nullptr);
    CHECK(time_then_depth != nullptr);
    CHECK_EQ(std::string(depth_then_time->name()),
             "alphabeta-t17-noorder");
    CHECK_EQ(std::string(time_then_depth->name()), "alphabeta-t17");
}

TEST(search_pruning_flags_have_canonical_names) {
    AlphaBetaAgent::Options no_null;
    no_null.null_move = false;
    CHECK_EQ(std::string(AlphaBetaAgent(no_null, 1).name()),
             "alphabeta-d4-nonull");

    AlphaBetaAgent::Options no_lmr;
    no_lmr.late_move_reductions = false;
    CHECK_EQ(std::string(AlphaBetaAgent(no_lmr, 1).name()),
             "alphabeta-d4-nolmr");

    AlphaBetaAgent::Options neither;
    neither.null_move = false;
    neither.late_move_reductions = false;
    CHECK_EQ(std::string(AlphaBetaAgent(neither, 1).name()),
             "alphabeta-d4-nonull-nolmr");

    auto factory_no_null = make_agent("alphabeta-d5-nonull", 2);
    auto factory_no_lmr = make_agent("alphabeta-d5-nolmr", 3);
    auto reversed_combination =
        make_agent("alphabeta-d2-t17-nolmr-nonull-noorder", 4);
    CHECK(factory_no_null != nullptr);
    CHECK(factory_no_lmr != nullptr);
    CHECK(reversed_combination != nullptr);
    CHECK_EQ(std::string(factory_no_null->name()),
             "alphabeta-d5-nonull");
    CHECK_EQ(std::string(factory_no_lmr->name()),
             "alphabeta-d5-nolmr");
    CHECK_EQ(std::string(reversed_combination->name()),
             "alphabeta-t17-noorder-nonull-nolmr");
}

TEST(disabling_search_pruning_preserves_the_frozen_depth_four_tree) {
    struct Fixture {
        const char* sfen;
        const char* move;
        uint64_t nodes;
    };
    constexpr Fixture fixtures[] = {
        {"rbsgk/4p/5/P4/KGSBR b - 1", "4e3d", 9930},
        {"rbg1k/2s2/3Bp/P1GR1/K1S2 b - 11", "2d2e", 9319},
        {"3rk/g1b2/2G1p/P2R1/K1S2 b Sb 19", "3c3b", 48932},
        {"3rk/g1G2/4p/P2R1/K1S2 w BSb 20", "2a2d", 109934},
        {"k4/5/2+P2/5/4K w RBGSrbgs 40", "R*4e", 852108},
        {"3pk/5/4G/5/K4 b PG 1", "G*1b", 8419},
    };

    for (const Fixture& fixture : fixtures) {
        AlphaBetaAgent::Options options;
        options.depth = 4;
        options.null_move = false;
        options.late_move_reductions = false;
        AlphaBetaAgent control(options, 99);
        control.new_game(99);

        const Move chosen = control.choose(game_of(fixture.sfen));
        CHECK_EQ(usi_move(chosen), std::string(fixture.move));
        CHECK_EQ(control.nodes(), fixture.nodes);
    }
}

TEST(search_pruning_is_deterministic_at_an_active_depth) {
    AlphaBetaAgent first(6, 17);
    AlphaBetaAgent second(6, 17);
    first.new_game(99);
    second.new_game(99);

    const Game game;
    const Move first_move = first.choose(game);
    const Move second_move = second.choose(game);
    CHECK_EQ(first_move, second_move);
    CHECK_EQ(first.nodes(), second.nodes());

    // This depth exercises both techniques in the start position; the
    // control's larger frozen tree makes the test fail if the two equal
    // runs merely bypass pruning.
    AlphaBetaAgent::Options control_options;
    control_options.depth = 6;
    control_options.null_move = false;
    control_options.late_move_reductions = false;
    AlphaBetaAgent control(control_options, 17);
    control.new_game(99);
    CHECK(control.choose(game) != MOVE_NONE);
    CHECK(first.nodes() < control.nodes());
}

TEST(null_move_is_never_searched_while_in_check) {
    const Position checked = position_of("r3k/5/5/5/K4 b - 1");
    CHECK(checked.in_check());

    AlphaBetaAgent::Options enabled;
    enabled.quiescence = false;
    enabled.transposition = false;
    enabled.move_ordering = false;
    enabled.late_move_reductions = false;
    AlphaBetaAgent with_null(enabled, 1);

    AlphaBetaAgent::Options disabled = enabled;
    disabled.null_move = false;
    AlphaBetaAgent without_null(disabled, 1);

    // At depth three only the checked root is eligible by depth; all
    // descendants are too shallow. Equal trees therefore prove that no
    // pass was attempted at the checked node.
    const int with_score =
        AlphaBetaAgentTestAccess::exact_score(with_null, checked, 3);
    const int without_score =
        AlphaBetaAgentTestAccess::exact_score(without_null, checked, 3);
    CHECK_EQ(with_score, without_score);
    CHECK_EQ(with_null.nodes(), without_null.nodes());
}

TEST(null_move_does_not_cut_a_mate_band_window) {
    const Position pos;

    AlphaBetaAgent::Options enabled;
    enabled.quiescence = false;
    enabled.transposition = false;
    enabled.move_ordering = false;
    enabled.late_move_reductions = false;
    AlphaBetaAgent with_null(enabled, 1);

    AlphaBetaAgent::Options disabled = enabled;
    disabled.null_move = false;
    AlphaBetaAgent without_null(disabled, 1);

    constexpr int alpha = -MATE_SCORE - 1;
    constexpr int beta = -MATE_THRESHOLD;
    const int with_score = AlphaBetaAgentTestAccess::window_score(
        with_null, pos, 3, alpha, beta);
    const int without_score = AlphaBetaAgentTestAccess::window_score(
        without_null, pos, 3, alpha, beta);
    CHECK_EQ(with_score, without_score);
    CHECK_EQ(with_null.nodes(), without_null.nodes());
}

TEST(timed_search_returns_the_last_complete_iteration) {
    Game game;

    AlphaBetaAgent fixed_depth_two(2, 9);
    fixed_depth_two.new_game(11);
    const Move expected = fixed_depth_two.choose(game);
    const uint64_t depth_two_nodes = fixed_depth_two.nodes();

    AlphaBetaAgent fixed_depth_three(3, 9);
    fixed_depth_three.new_game(11);
    fixed_depth_three.choose(game);
    const uint64_t depth_three_nodes = fixed_depth_three.nodes();

    const uint64_t interval =
        AlphaBetaAgentTestAccess::time_check_interval();
    const uint64_t timeout_poll =
        (2 * depth_two_nodes / interval + 1) * interval;
    CHECK(timeout_poll < depth_three_nodes);

    AlphaBetaAgent::Options options;
    // Under the test clock one millisecond equals one visited node.
    // The chosen poll lies after twice the completed depth-two work, so
    // the half-budget gate permits depth three, which then aborts at
    // exactly that poll.
    options.movetime_ms = static_cast<int>(timeout_poll);
    AlphaBetaAgent timed(options, 9);
    AlphaBetaAgentTestAccess::use_node_clock(timed);
    timed.new_game(11);
    const Move actual = timed.choose(game);

    CHECK(AlphaBetaAgentTestAccess::timed_out(timed));
    CHECK_EQ(AlphaBetaAgentTestAccess::completed_depth(timed), 2);
    CHECK_EQ(actual, expected);

    MoveList legal;
    generate_legal(game.position(), legal);
    CHECK(legal.contains(actual));
}

TEST(fixed_depth_search_never_reads_the_wall_clock) {
    AlphaBetaAgent fixed(4, 1);
    int clock_calls = 0;
    AlphaBetaAgentTestAccess::count_clock_calls(fixed, clock_calls);
    fixed.new_game(7);
    CHECK(fixed.choose(Game()) != MOVE_NONE);
    CHECK_EQ(clock_calls, 0);
}

TEST(timed_search_stops_early_after_a_proven_mate) {
    AlphaBetaAgent::Options options;
    options.movetime_ms = 1000000;
    AlphaBetaAgent timed(options, 1);
    AlphaBetaAgentTestAccess::use_node_clock(timed);
    timed.new_game(7);

    const Game mate_in_one = game_of("k2r1/5/1B3/5/4K b G 1");
    CHECK_EQ(usi_move(timed.choose(mate_in_one)), std::string("G*5b"));
    CHECK_EQ(AlphaBetaAgentTestAccess::completed_depth(timed), 1);
    CHECK(!AlphaBetaAgentTestAccess::timed_out(timed));
}

TEST(real_movetime_returns_promptly_with_a_legal_move) {
    constexpr int BUDGET_MS = 100;
    AlphaBetaAgent::Options options;
    options.movetime_ms = BUDGET_MS;
    AlphaBetaAgent timed(options, 1);
    timed.new_game(7);
    Game game;

    const auto start = std::chrono::steady_clock::now();
    const Move move = timed.choose(game);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    MoveList legal;
    generate_legal(game.position(), legal);
    CHECK(legal.contains(move));
    CHECK(AlphaBetaAgentTestAccess::completed_depth(timed) > 0);
    CHECK(elapsed < std::chrono::milliseconds(BUDGET_MS * 2));
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

TEST(mcts_finds_mate_in_one_given_a_reasonable_budget) {
    Game game = game_of("k2r1/5/1B3/5/4K b G 1");
    for (uint64_t trial = 0; trial < 5; ++trial) {
        MctsAgent agent(400, trial);
        CHECK_EQ(usi_move(agent.choose(game)), std::string("G*5b"));
    }
}

TEST(mcts_beats_random_and_greedy) {
    auto random_agent = make_agent("random", 2);
    MctsAgent mcts(200, 1);
    const MatchResult vs_random = play_match(mcts, *random_agent, 20, 21);
    CHECK(vs_random.score_a() > 0.85);
    CHECK_EQ(vs_random.illegal, 0);
}

TEST(more_playouts_beat_fewer) {
    // The MCTS counterpart of deeper-beats-shallower: if extra budget
    // does not help, the tree is not doing anything.
    MctsAgent rich(800, 1), poor(200, 2);
    const MatchResult result = play_match(rich, poor, 20, 31);
    CHECK_EQ(result.illegal, 0);
    CHECK(result.score_a() > 0.65);
}

TEST(alphabeta_outplays_mcts_in_this_game) {
    // Recorded because it is a finding, not an accident: random playouts
    // carry almost no signal in a game where dropped pieces make every
    // position sharp, so plain UCT loses badly to a shallow material
    // search. Replacing rollouts with a learned value is what phase 3 is
    // for; if that ever lands, this test is the thing that should flip.
    auto search = make_agent("alphabeta-d3", 1);
    MctsAgent mcts(400, 2);
    const MatchResult result = play_match(*search, mcts, 20, 41);
    CHECK(result.score_a() > 0.85);
}
