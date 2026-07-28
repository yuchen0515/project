// Static evaluation, in centipawns from the side to move's point of view.

#pragma once

#include <array>

#include "../core/position.h"

namespace pioneer {

// Comfortably beyond any material score, and far enough below INT32_MAX
// that adding ply offsets cannot overflow.
constexpr int MATE_SCORE = 30000;
constexpr int MAX_EVAL = 10000;

// Ordered by mobility on a 5x5 board, which is what actually matters
// here: the rook and bishop sweep it end to end, while a pawn moves one
// square and is mostly worth the tempo of dropping it somewhere useful.
// The legacy engine had these backwards (rook 54 below gold 74), which
// is enough on its own to make an otherwise sound search play badly.
constexpr int PIECE_VALUE[PIECE_TYPE_NB] = {
    0,     // none
    100,   // pawn
    340,   // silver
    500,   // bishop
    570,   // rook
    380,   // gold
    0,     // king - presence is decided by mate detection, not material
    390,   // promoted pawn   (moves as gold)
    390,   // promoted silver (moves as gold)
    640,   // horse
    720,   // dragon
};

// A piece in hand can be dropped anywhere, which is worth more than the
// same piece committed to a square.
constexpr int HAND_BONUS_PERCENT = 10;

// Each term as a percentage, so a term can be switched on and the
// difference measured in the arena rather than argued about.
//
// Placement and king safety default to off, and that is a measured
// decision, not an oversight: with them on the engine wins self-play by
// +228 elo and loses about 230 against an outside opponent. See
// docs/agents.md - hand-picked positional weights turned out to exploit
// an opponent that shares them rather than to understand the game.
struct EvalWeights {
    int material = 100;
    int placement = 0;
    int king_safety = 0;
};

inline constexpr EvalWeights DEFAULT_WEIGHTS{};

// The board has no left-right symmetry between the players: the two
// sides start rotated 180 degrees from each other, not reflected. So the
// mirror of a square flips both rank and file.
constexpr Square mirror(Square s) {
    return make_square(RANK_NB - 1 - rank_of(s), FILE_NB - 1 - col_of(s));
}

int evaluate(const Position& pos, const EvalWeights& weights = DEFAULT_WEIGHTS);

}  // namespace pioneer
