#include "eval.h"

namespace pioneer {

int evaluate(const Position& pos) {
    int score = 0;

    for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        const int value = PIECE_VALUE[pt];
        if (!value) continue;
        score += value * popcount(pos.pieces(BLACK, PieceType(pt)));
        score -= value * popcount(pos.pieces(WHITE, PieceType(pt)));
    }

    for (int i = 0; i < HAND_TYPE_NB; ++i) {
        const PieceType pt = hand_type(i);
        const int value =
            PIECE_VALUE[pt] * (100 + HAND_BONUS_PERCENT) / 100;
        score += value * pos.hand_count(BLACK, pt);
        score -= value * pos.hand_count(WHITE, pt);
    }

    return pos.side_to_move() == BLACK ? score : -score;
}

}  // namespace pioneer
