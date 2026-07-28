// Uniformly random legal move. Useless as an opponent, essential as a
// baseline: it is the zero point every rating is measured from, and any
// agent that fails to beat it decisively is broken.

#pragma once

#include <random>

#include "agent.h"

namespace pioneer {

// Mixes an agent's own seed with the per-game seed so two agents sharing
// a game seed do not draw correlated streams.
constexpr uint64_t mix_seeds(uint64_t a, uint64_t b) {
    uint64_t z = a + b * 0x9E3779B97F4A7C15ull;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

class RandomAgent : public Agent {
   public:
    explicit RandomAgent(uint64_t seed = 0x5EED)
        : seed_(seed), rng_(mix_seeds(seed, 0)) {}

    const char* name() const override { return "random"; }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override { rng_.seed(mix_seeds(seed_, seed)); }

   private:
    uint64_t seed_;
    std::mt19937_64 rng_;
};

}  // namespace pioneer
