// Static evaluation, in centipawns from the side to move's point of view.

#pragma once

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

int evaluate(const Position& pos);

}  // namespace pioneer
