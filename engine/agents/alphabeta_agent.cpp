#include "alphabeta_agent.h"

#include <algorithm>
#include <vector>

#include "eval.h"

namespace pioneer {

namespace {

// Most valuable victim, least valuable attacker: try the captures that
// are most likely to be good first, so alpha-beta cuts off sooner. The
// ordering only affects speed, never the result.
int legacy_move_order_key(const Position& pos, Move m, Move tt_move) {
    // Whatever was best here last time is the best guess this time, and
    // it costs one comparison to try it first.
    if (m == tt_move) return 1 << 20;

    if (is_drop(m)) return 0;

    const Piece captured = pos.piece_on(to_sq(m));
    const int victim = captured == NO_PIECE ? 0 : PIECE_VALUE[type_of(captured)];
    const int attacker = PIECE_VALUE[type_of(pos.piece_on(from_sq(m)))];
    const int promotion = is_promotion(m) ? 50 : 0;

    return victim ? victim * 16 - attacker + promotion : promotion;
}

bool is_capture(const Position& pos, Move m) {
    return !is_drop(m) && pos.piece_on(to_sq(m)) != NO_PIECE;
}

bool is_quiet(const Position& pos, Move m) {
    return !is_capture(pos, m) && !is_promotion(m);
}

struct MoveOrderKey {
    int tier;
    int detail;
};

bool operator>(MoveOrderKey a, MoveOrderKey b) {
    return a.tier != b.tier ? a.tier > b.tier : a.detail > b.detail;
}

std::string describe(const AlphaBetaAgent::Options& options) {
    std::string name = "alphabeta-d" + std::to_string(options.depth);
    if (!options.quiescence) name += "-noqs";
    if (!options.transposition) name += "-nott";
    if (!options.move_ordering) name += "-noorder";
    if (options.weights.placement) name += "+pst";
    if (options.weights.king_safety) name += "+ks";
    return name;
}

}  // namespace

AlphaBetaAgent::AlphaBetaAgent(Options options, uint64_t seed)
    : options_(options),
      seed_(seed),
      name_(describe(options)),
      rng_(mix_seeds(seed, 0)),
      table_(options.transposition ? options.table_mb : 1) {
    options_.depth = std::max(1, options_.depth);
    killers_.assign(size_t(options_.depth + 1),
                    std::array<Move, KILLER_SLOT_NB>{MOVE_NONE, MOVE_NONE});
}

AlphaBetaAgent::AlphaBetaAgent(int depth, uint64_t seed, bool quiescence)
    : AlphaBetaAgent(
          [&] {
              Options options;
              options.depth = std::max(1, depth);
              options.quiescence = quiescence;
              return options;
          }(),
          seed) {}

void AlphaBetaAgent::new_game(uint64_t seed) {
    rng_.seed(mix_seeds(seed_, seed));
    // Carrying entries between games would make a game's result depend
    // on the games before it, which costs reproducibility for very
    // little search value.
    table_.clear();
    for (auto& slots : killers_) slots.fill(MOVE_NONE);
    history_ = {};
}

void AlphaBetaAgent::order_moves(const Position& pos, const MoveList& moves,
                                 Move tt_move, int ply, bool use_heuristics,
                                 std::vector<Move>& out) const {
    out.assign(moves.begin(), moves.end());

    // The control variant and quiescence both retain the exact pre-6.2
    // ordering: TT move, then MVV-LVA with the existing promotion bonus.
    if (!use_heuristics) {
        std::stable_sort(out.begin(), out.end(), [&](Move a, Move b) {
            return legacy_move_order_key(pos, a, tt_move) >
                   legacy_move_order_key(pos, b, tt_move);
        });
        return;
    }

    auto key = [&](Move m) {
        if (m == tt_move) return MoveOrderKey{4, 0};

        // Captures and quiet promotions are tactical. Test this before
        // killers: a move remembered as quiet may be a capture now.
        if (is_capture(pos, m) || is_promotion(m))
            return MoveOrderKey{
                3, legacy_move_order_key(pos, m, MOVE_NONE)};

        if (ply >= 0 && size_t(ply) < killers_.size()) {
            if (m == killers_[ply][0]) return MoveOrderKey{2, 2};
            if (m == killers_[ply][1]) return MoveOrderKey{2, 1};
        }

        return MoveOrderKey{
            1, history_[pos.side_to_move()][origin_field(m)][to_sq(m)]};
    };

    std::stable_sort(out.begin(), out.end(),
                     [&](Move a, Move b) { return key(a) > key(b); });
}

void AlphaBetaAgent::record_quiet_cutoff(const Position& pos, Move m, int depth,
                                         int ply) {
    if (!options_.move_ordering || !is_quiet(pos, m)) return;

    if (ply >= 0 && size_t(ply) < killers_.size() &&
        killers_[ply][0] != m) {
        killers_[ply][1] = killers_[ply][0];
        killers_[ply][0] = m;
    }

    const int capped_depth = std::min(std::max(depth, 1), 32);
    const int bonus = capped_depth * capped_depth;
    uint16_t& score =
        history_[pos.side_to_move()][origin_field(m)][to_sq(m)];
    score = uint16_t(std::min(HISTORY_MAX, int(score) + bonus));
}

void AlphaBetaAgent::age_history() {
    if (!options_.move_ordering) return;
    for (auto& by_origin : history_)
        for (auto& by_target : by_origin)
            for (uint16_t& score : by_target) score /= 2;
}

// Evaluating in the middle of an exchange reads whatever the last
// capture left behind, so the search keeps going until the position is
// quiet. Without this, a depth-N search happily hangs a rook on ply N+1.
int AlphaBetaAgent::quiesce(const Position& pos, int alpha, int beta, int ply) {
    ++nodes_;

    MoveList moves;
    generate_legal(pos, moves);
    if (moves.empty()) return -MATE_SCORE + ply;
    if (ply >= MAX_QUIESCENCE_PLY) return evaluate(pos, options_.weights);

    // In check there is nothing quiet about the position and every
    // evasion has to be considered - standing pat would assume an
    // option the side to move does not have.
    const bool in_check = pos.in_check();

    int best = -MATE_SCORE - 1;
    if (!in_check) {
        best = evaluate(pos, options_.weights);
        if (best >= beta) return best;
        if (best > alpha) alpha = best;
    }

    std::vector<Move> ordered;
    order_moves(pos, moves, MOVE_NONE, ply, false, ordered);

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

    const int original_alpha = alpha;
    Move tt_move = MOVE_NONE;

    if (options_.transposition) {
        if (const TTEntry* entry = table_.probe(pos.key())) {
            tt_move = entry->move;
            const int score = score_from_tt(entry->score, ply);

            // A shallower result is still a usable move hint, but its
            // score cannot stand in for a deeper search.
            if (entry->depth >= depth) {
                if (entry->bound == Bound::EXACT) return score;
                if (entry->bound == Bound::LOWER && score >= beta) return score;
                if (entry->bound == Bound::UPPER && score <= alpha) return score;
            }
        }
    }

    MoveList moves;
    generate_legal(pos, moves);

    // No legal move loses, in check or not - shogi scores stalemate as a
    // loss too. Subtracting the ply prefers mates that arrive sooner and
    // makes the engine prolong its own losses.
    if (moves.empty()) return -MATE_SCORE + ply;

    if (depth <= 0)
        return options_.quiescence ? quiesce(pos, alpha, beta, ply)
                                   : evaluate(pos, options_.weights);

    std::vector<Move> ordered;
    order_moves(pos, moves, tt_move, ply, options_.move_ordering, ordered);

    int best = -MATE_SCORE - 1;
    Move best_move = MOVE_NONE;

    for (Move m : ordered) {
        Position next = pos;
        next.apply(m);
        const int score = -search(next, depth - 1, -beta, -alpha, ply + 1);

        if (score > best) {
            best = score;
            best_move = m;
        }
        if (best > alpha) alpha = best;
        if (alpha >= beta) {
            record_quiet_cutoff(pos, m, depth, ply);
            break;  // opponent will not allow this line
        }
    }

    if (options_.transposition) {
        const Bound bound = best <= original_alpha ? Bound::UPPER
                            : best >= beta         ? Bound::LOWER
                                                   : Bound::EXACT;
        table_.store(pos.key(), depth, best, bound, best_move, ply);
    }
    return best;
}

Move AlphaBetaAgent::choose(const Game& game) {
    nodes_ = 0;
    age_history();

    MoveList moves;
    generate_legal(game.position(), moves);
    if (moves.empty()) return MOVE_NONE;

    std::vector<Move> best_moves;
    Move previous_best = MOVE_NONE;

    // Iterative deepening. It costs a fraction of the total nodes and
    // pays for itself: each pass leaves the table primed, so the next
    // one tries the previously best move first and cuts off sooner.
    for (int depth = 1; depth <= options_.depth; ++depth) {
        std::vector<Move> ordered;
        order_moves(game.position(), moves, previous_best, 0,
                    options_.move_ordering, ordered);

        int best = -MATE_SCORE - 1;
        best_moves.clear();

        for (Move m : ordered) {
            Position next = game.position();
            next.apply(m);
            // Full window at the root: a tightened one would return a
            // bound rather than a score, and equal-scoring moves could
            // not be collected for the tie-break below.
            const int score =
                -search(next, depth - 1, -MATE_SCORE - 1, MATE_SCORE + 1, 1);

            if (score > best) {
                best = score;
                best_moves.assign(1, m);
            } else if (score == best) {
                best_moves.push_back(m);
            }
        }
        previous_best = best_moves.front();
    }

    std::uniform_int_distribution<size_t> pick(0, best_moves.size() - 1);
    return best_moves[pick(rng_)];
}

}  // namespace pioneer
