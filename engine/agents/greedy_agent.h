// One ply deep: plays the move leaving the best material balance, and
// takes a mate when one is on the board.
//
// It is the cheapest agent that plays recognisable moves, which makes it
// the second rung of the ladder after random - and a useful control. A
// deeper search that cannot beat this convincingly is not searching.

#pragma once

#include <random>

#include "agent.h"
#include "random_agent.h"  // mix_seeds

namespace pioneer {

class GreedyAgent : public Agent {
   public:
    explicit GreedyAgent(uint64_t seed = 0x6EED)
        : seed_(seed), rng_(mix_seeds(seed, 0)) {}

    const char* name() const override { return "greedy"; }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override { rng_.seed(mix_seeds(seed_, seed)); }

   private:
    uint64_t seed_;
    std::mt19937_64 rng_;
};

}  // namespace pioneer
