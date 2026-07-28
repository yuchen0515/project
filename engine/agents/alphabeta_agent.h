// Fixed-depth negamax with alpha-beta pruning.
//
// The first agent here that actually searches. Everything it needs is
// already verified - legal moves, a material evaluation, and an arena to
// prove the result - so any strength it shows is real.

#pragma once

#include <random>
#include <string>

#include "agent.h"
#include "random_agent.h"  // mix_seeds

namespace pioneer {

class AlphaBetaAgent : public Agent {
   public:
    explicit AlphaBetaAgent(int depth = 4, uint64_t seed = 0x7EED);

    const char* name() const override { return name_.c_str(); }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override { rng_.seed(mix_seeds(seed_, seed)); }

    // Nodes visited by the last choose(), for measuring the effect of
    // pruning and ordering changes.
    uint64_t nodes() const { return nodes_; }

   private:
    int search(const Position& pos, int depth, int alpha, int beta, int ply);

    int depth_;
    uint64_t seed_;
    std::string name_;
    std::mt19937_64 rng_;
    uint64_t nodes_ = 0;
};

}  // namespace pioneer
