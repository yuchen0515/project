// Monte Carlo tree search with UCT.
//
// This is the algorithm the original 2021 project set out to build. That
// version never advanced the position inside the tree, expanded children
// against their parent's move list, and mixed a static capture score
// into backpropagation on an asymmetric scale (see legacy/README.md).
// Getting those three things right is most of what this file is.

#pragma once

#include <random>
#include <string>
#include <vector>

#include "agent.h"
#include "random_agent.h"  // mix_seeds

namespace pioneer {

// Exploration constant for UCT with values in [-1, 1]. A starting point
// rather than a tuned one: the arena can settle it by measurement.
constexpr double UCT_EXPLORATION = 1.4;

// Playouts are cut off rather than played to the end. Random play in
// minishogi wanders for a long time, and a material evaluation of the
// cut-off position carries more signal than the eventual random result.
constexpr int ROLLOUT_DEPTH = 24;

// Centipawns per unit of tanh, mapping evaluations onto [-1, 1] so a
// playout result and a terminal result live on the same scale.
constexpr double EVAL_SCALE = 400.0;

class MctsAgent : public Agent {
   public:
    explicit MctsAgent(int simulations = 1000, uint64_t seed = 0x8EED);

    const char* name() const override { return name_.c_str(); }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override { rng_.seed(mix_seeds(seed_, seed)); }

   private:
    struct Node {
        Move move = MOVE_NONE;  // the move that led to this node
        int visits = 0;
        // Sum of results from the point of view of the side to move at
        // this node. A child's value is therefore the opponent's, and
        // selection negates it - the sign discipline the old engine lost.
        double value = 0.0;
        std::vector<Node> children;
    };

    double simulate(Node& node, Position pos);
    Node* select_child(Node& parent);
    double rollout(Position pos);

    int simulations_;
    uint64_t seed_;
    std::string name_;
    std::mt19937_64 rng_;
};

}  // namespace pioneer
