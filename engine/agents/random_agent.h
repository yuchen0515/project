// Uniformly random legal move. Useless as an opponent, essential as a
// baseline: it is the zero point every rating is measured from, and any
// agent that fails to beat it decisively is broken.

#pragma once

#include <random>

#include "agent.h"

namespace pioneer {

class RandomAgent : public Agent {
   public:
    explicit RandomAgent(uint64_t seed = 0x5EED) : seed_(seed), rng_(seed) {}

    const char* name() const override { return "random"; }
    Move choose(const Game& game) override;
    void new_game() override { rng_.seed(seed_); }

   private:
    uint64_t seed_;
    std::mt19937_64 rng_;
};

}  // namespace pioneer
