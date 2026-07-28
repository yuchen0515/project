// Iteratively deepened negamax with alpha-beta, a transposition table
// and a quiescence search.
//
// Everything it needs is already verified - legal moves, a material
// evaluation, and an arena to prove the result - so any strength it
// shows is real.

#pragma once

#include <random>
#include <string>

#include "agent.h"
#include "random_agent.h"  // mix_seeds
#include "transposition.h"

namespace pioneer {

// Depth at which quiescence stops chasing captures. Captures strictly
// reduce the pieces on the board so the recursion terminates on its own;
// this only guards against a pathological position.
constexpr int MAX_QUIESCENCE_PLY = 32;

class AlphaBetaAgent : public Agent {
   public:
    struct Options {
        int depth = 4;
        bool quiescence = true;
        // Off is the control variant used to measure what each feature
        // is worth; neither is something you would play with.
        bool transposition = true;
        size_t table_mb = 16;
    };

    explicit AlphaBetaAgent(int depth = 4, uint64_t seed = 0x7EED,
                            bool quiescence = true);
    AlphaBetaAgent(Options options, uint64_t seed);

    const char* name() const override { return name_.c_str(); }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override;

    // Nodes visited by the last choose(), for measuring the effect of
    // pruning and ordering changes.
    uint64_t nodes() const { return nodes_; }

   private:
    int search(const Position& pos, int depth, int alpha, int beta, int ply);
    int quiesce(const Position& pos, int alpha, int beta, int ply);

    Options options_;
    uint64_t seed_;
    std::string name_;
    std::mt19937_64 rng_;
    TranspositionTable table_;
    uint64_t nodes_ = 0;
};

}  // namespace pioneer
