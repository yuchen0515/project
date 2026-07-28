#include "greedy_agent.h"

#include <vector>

#include "eval.h"

namespace pioneer {

Move GreedyAgent::choose(const Game& game) {
    MoveList moves;
    generate_legal(game.position(), moves);
    if (moves.empty()) return MOVE_NONE;

    int best = -MATE_SCORE - 1;
    std::vector<Move> best_moves;

    for (Move m : moves) {
        Position next = game.position();
        next.apply(m);

        // evaluate() speaks for the side to move, which after our move
        // is the opponent - hence the negation.
        const int score = is_checkmate(next) ? MATE_SCORE : -evaluate(next);

        if (score > best) {
            best = score;
            best_moves.assign(1, m);
        } else if (score == best) {
            best_moves.push_back(m);
        }
    }

    // Ties are broken randomly rather than by generation order. Without
    // this, greedy against greedy plays one fixed game forever and the
    // repetition rule decides it.
    std::uniform_int_distribution<size_t> pick(0, best_moves.size() - 1);
    return best_moves[pick(rng_)];
}

}  // namespace pioneer
