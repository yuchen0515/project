#include "mcts_agent.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "eval.h"

namespace pioneer {

MctsAgent::MctsAgent(int simulations, uint64_t seed)
    : simulations_(std::max(1, simulations)),
      seed_(seed),
      name_("mcts-s" + std::to_string(std::max(1, simulations))),
      rng_(mix_seeds(seed, 0)) {}

double MctsAgent::rollout(Position pos) {
    const Color root = pos.side_to_move();

    for (int i = 0; i < ROLLOUT_DEPTH; ++i) {
        MoveList moves;
        generate_legal(pos, moves);
        if (moves.empty())  // the side to move here has lost
            return pos.side_to_move() == root ? -1.0 : 1.0;

        std::uniform_int_distribution<int> pick(0, moves.size() - 1);
        pos.apply(moves[pick(rng_)]);
    }

    const int score = evaluate(pos);  // speaks for pos.side_to_move()
    const double signed_score = pos.side_to_move() == root ? score : -score;
    return std::tanh(signed_score / EVAL_SCALE);
}

MctsAgent::Node* MctsAgent::select_child(Node& parent) {
    // Unvisited children first, in random order, so the tie is not
    // broken by move generation order.
    std::vector<Node*> unvisited;
    for (Node& child : parent.children)
        if (child.visits == 0) unvisited.push_back(&child);

    if (!unvisited.empty()) {
        std::uniform_int_distribution<size_t> pick(0, unvisited.size() - 1);
        return unvisited[pick(rng_)];
    }

    const double log_parent = std::log(double(parent.visits));
    Node* best = nullptr;
    double best_uct = -std::numeric_limits<double>::max();

    for (Node& child : parent.children) {
        // The child's value belongs to the opponent, so it is negated
        // before it can be compared as our gain.
        const double exploit = -child.value / child.visits;
        const double explore =
            UCT_EXPLORATION * std::sqrt(log_parent / child.visits);
        if (const double uct = exploit + explore; uct > best_uct) {
            best_uct = uct;
            best = &child;
        }
    }
    return best;
}

double MctsAgent::simulate(Node& node, Position pos) {
    if (node.children.empty()) {
        MoveList moves;
        generate_legal(pos, moves);

        if (moves.empty()) {
            // Terminal: the side to move here has lost, and will keep
            // losing every time the tree revisits this node.
            node.visits += 1;
            node.value += -1.0;
            return -1.0;
        }

        for (Move m : moves) {
            Node child;
            child.move = m;
            node.children.push_back(child);
        }

        const double result = rollout(pos);
        node.visits += 1;
        node.value += result;
        return result;
    }

    Node* child = select_child(node);
    Position next = pos;
    next.apply(child->move);  // the position really does advance here

    const double result = -simulate(*child, next);
    node.visits += 1;
    node.value += result;
    return result;
}

Move MctsAgent::choose(const Game& game) {
    MoveList moves;
    generate_legal(game.position(), moves);
    if (moves.empty()) return MOVE_NONE;
    if (moves.size() == 1) return moves[0];

    Node root;
    for (int i = 0; i < simulations_; ++i) simulate(root, game.position());

    // Most visited rather than best average: visit counts are what the
    // search actually committed its budget to, and they are far less
    // noisy than a mean over a handful of playouts.
    const Node* best = &root.children.front();
    for (const Node& child : root.children)
        if (child.visits > best->visits) best = &child;
    return best->move;
}

}  // namespace pioneer
