#include "alphabeta_agent.h"

#include <algorithm>
#include <vector>

#include "eval.h"

namespace pioneer {

namespace {

// Most valuable victim, least valuable attacker: try the captures that
// are most likely to be good first, so alpha-beta cuts off sooner. The
// ordering only affects speed, never the result.
int move_order_key(const Position& pos, Move m) {
    if (is_drop(m)) return 0;

    const Piece captured = pos.piece_on(to_sq(m));
    const int victim = captured == NO_PIECE ? 0 : PIECE_VALUE[type_of(captured)];
    const int attacker = PIECE_VALUE[type_of(pos.piece_on(from_sq(m)))];
    const int promotion = is_promotion(m) ? 50 : 0;

    return victim ? victim * 16 - attacker + promotion : promotion;
}

void order_moves(const Position& pos, MoveList& moves, std::vector<Move>& out) {
    out.assign(moves.begin(), moves.end());
    std::stable_sort(out.begin(), out.end(), [&](Move a, Move b) {
        return move_order_key(pos, a) > move_order_key(pos, b);
    });
}

bool is_capture(const Position& pos, Move m) {
    return !is_drop(m) && pos.piece_on(to_sq(m)) != NO_PIECE;
}

}  // namespace

AlphaBetaAgent::AlphaBetaAgent(int depth, uint64_t seed, bool quiescence)
    : depth_(std::max(1, depth)),
      seed_(seed),
      quiescence_(quiescence),
      name_("alphabeta-d" + std::to_string(std::max(1, depth)) +
            (quiescence ? "" : "-noqs")),
      rng_(mix_seeds(seed, 0)) {}

// Evaluating in the middle of an exchange reads whatever the last
// capture left behind, so the search keeps going until the position is
// quiet. Without this, a depth-N search happily hangs a rook on ply N+1.
int AlphaBetaAgent::quiesce(const Position& pos, int alpha, int beta, int ply) {
    ++nodes_;

    MoveList moves;
    generate_legal(pos, moves);
    if (moves.empty()) return -MATE_SCORE + ply;
    if (ply >= MAX_QUIESCENCE_PLY) return evaluate(pos);

    // In check there is nothing quiet about the position and every
    // evasion has to be considered - standing pat would assume an
    // option the side to move does not have.
    const bool in_check = pos.in_check();

    int best = -MATE_SCORE - 1;
    if (!in_check) {
        best = evaluate(pos);
        if (best >= beta) return best;
        if (best > alpha) alpha = best;
    }

    std::vector<Move> ordered;
    order_moves(pos, moves, ordered);

    for (Move m : ordered) {
        if (!in_check && !is_capture(pos, m)) continue;

        Position next = pos;
        next.apply(m);
        const int score = -quiesce(next, -beta, -alpha, ply + 1);

        if (score > best) best = score;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
    }
    return best;
}

int AlphaBetaAgent::search(const Position& pos, int depth, int alpha, int beta,
                           int ply) {
    ++nodes_;

    MoveList moves;
    generate_legal(pos, moves);

    // No legal move loses, in check or not - shogi scores stalemate as a
    // loss too. Subtracting the ply prefers mates that arrive sooner and
    // makes the engine prolong its own losses.
    if (moves.empty()) return -MATE_SCORE + ply;

    if (depth <= 0)
        return quiescence_ ? quiesce(pos, alpha, beta, ply) : evaluate(pos);

    std::vector<Move> ordered;
    order_moves(pos, moves, ordered);

    int best = -MATE_SCORE - 1;
    for (Move m : ordered) {
        Position next = pos;
        next.apply(m);
        const int score = -search(next, depth - 1, -beta, -alpha, ply + 1);

        if (score > best) best = score;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;  // opponent will not allow this line
    }
    return best;
}

Move AlphaBetaAgent::choose(const Game& game) {
    nodes_ = 0;

    MoveList moves;
    generate_legal(game.position(), moves);
    if (moves.empty()) return MOVE_NONE;

    std::vector<Move> ordered;
    order_moves(game.position(), moves, ordered);

    int best = -MATE_SCORE - 1;
    std::vector<Move> best_moves;

    for (Move m : ordered) {
        Position next = game.position();
        next.apply(m);
        // Full window at the root: a tightened one would return a bound
        // rather than a score, and equal-scoring moves could not be
        // collected for the tie-break below.
        const int score =
            -search(next, depth_ - 1, -MATE_SCORE - 1, MATE_SCORE + 1, 1);

        if (score > best) {
            best = score;
            best_moves.assign(1, m);
        } else if (score == best) {
            best_moves.push_back(m);
        }
    }

    std::uniform_int_distribution<size_t> pick(0, best_moves.size() - 1);
    return best_moves[pick(rng_)];
}

}  // namespace pioneer
