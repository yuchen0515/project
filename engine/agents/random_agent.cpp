#include "random_agent.h"

namespace pioneer {

Move RandomAgent::choose(const Game& game) {
    MoveList moves;
    generate_legal(game.position(), moves);
    if (moves.empty()) return MOVE_NONE;

    // uniform_int_distribution rather than a modulo, which would bias
    // toward low indices - it matters here because this agent is the
    // measuring stick for everything else.
    std::uniform_int_distribution<int> pick(0, moves.size() - 1);
    return moves[pick(rng_)];
}

}  // namespace pioneer
