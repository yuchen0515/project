// Iteratively deepened negamax with alpha-beta, a transposition table
// and a quiescence search.
//
// Everything it needs is already verified - legal moves, a material
// evaluation, and an arena to prove the result - so any strength it
// shows is real.

#pragma once

#include <array>
#include <chrono>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "agent.h"
#include "random_agent.h"  // mix_seeds
#include "transposition.h"

namespace pioneer {

struct AlphaBetaAgentTestAccess;

// Depth at which quiescence stops chasing captures. Captures strictly
// reduce the pieces on the board so the recursion terminates on its own;
// this only guards against a pathological position.
constexpr int MAX_QUIESCENCE_PLY = 32;

class AlphaBetaAgent : public Agent {
   public:
    struct Options {
        int depth = 4;
        // Zero selects the reproducible fixed-depth mode. A positive
        // value searches successively deeper until this wall-clock
        // budget expires.
        int movetime_ms = 0;
        bool quiescence = true;
        // Off is the control variant used to measure what each feature
        // is worth; neither is something you would play with.
        bool transposition = true;
        bool move_ordering = true;
        size_t table_mb = 16;
        EvalWeights weights;
    };

    explicit AlphaBetaAgent(int depth = 4, uint64_t seed = 0x7EED,
                            bool quiescence = true);
    AlphaBetaAgent(Options options, uint64_t seed);

    const char* name() const override { return name_.c_str(); }
    Move choose(const Game& game) override;
    Move choose(const Game& game, int movetime_ms) override;
    void new_game(uint64_t seed) override;

    // Nodes visited by the last choose(), for measuring the effect of
    // pruning and ordering changes.
    uint64_t nodes() const { return nodes_; }

   private:
    static constexpr int KILLER_SLOT_NB = 2;
    static constexpr int HISTORY_ORIGIN_NB = DROP_ORIGIN + HAND_TYPE_NB;
    static constexpr int HISTORY_MAX = 65535;
    static constexpr int MAX_TIMED_DEPTH = 64;
    static constexpr uint64_t TIME_CHECK_INTERVAL = 2048;

    using KillerTable = std::vector<std::array<Move, KILLER_SLOT_NB>>;
    using HistoryTable =
        std::array<std::array<std::array<uint16_t, SQUARE_NB>,
                              HISTORY_ORIGIN_NB>,
                   COLOR_NB>;
    using SearchClock = std::chrono::steady_clock;
    using TimeSource = std::function<SearchClock::time_point()>;

    struct SearchTimeout {};

    friend struct AlphaBetaAgentTestAccess;

    Move choose_with_budget(const Game& game, int movetime_ms);
    void count_node();
    int search(const Position& pos, int depth, int alpha, int beta, int ply);
    int quiesce(const Position& pos, int alpha, int beta, int ply);
    void order_moves(const Position& pos, const MoveList& moves, Move tt_move,
                     int ply, bool use_heuristics,
                     std::vector<Move>& out) const;
    void record_quiet_cutoff(const Position& pos, Move m, int depth, int ply);
    void age_history();

    Options options_;
    uint64_t seed_;
    std::string name_;
    std::mt19937_64 rng_;
    TranspositionTable table_;
    KillerTable killers_;
    HistoryTable history_{};
    uint64_t nodes_ = 0;
    TimeSource now_ = [] { return SearchClock::now(); };
    SearchClock::time_point deadline_{};
    SearchClock::time_point half_deadline_{};
    bool timing_active_ = false;
    bool timed_out_ = false;
    int completed_depth_ = 0;
};

}  // namespace pioneer
