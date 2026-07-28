#include "eval.h"

#include "../core/bitboard.h"

namespace pioneer {

namespace {

// Placement tables, written from Black's point of view with rank a (the
// promotion zone) at the top, and mirrored for White. Small numbers on
// purpose: material decides most positions on a board this size, and an
// over-confident placement term just distorts the search.
using Table = std::array<int, SQUARE_NB>;

constexpr Table table_from(const int (&rows)[RANK_NB][FILE_NB]) {
    Table t{};
    for (int r = 0; r < RANK_NB; ++r)
        for (int c = 0; c < FILE_NB; ++c) t[r * FILE_NB + c] = rows[r][c];
    return t;
}

// A pawn one step from promoting is worth far more than one at home.
constexpr Table PAWN_TABLE = table_from({
    {  0,   0,   0,   0,   0},
    { 34,  38,  40,  38,  34},
    { 16,  20,  24,  20,  16},
    {  4,   6,   8,   6,   4},
    {  0,   0,   0,   0,   0},
});

// Generals are worth advancing, but not off the back rank early: they
// are also what stands between a drop and the king.
constexpr Table GENERAL_TABLE = table_from({
    { 10,  16,  18,  16,  10},
    { 12,  20,  24,  20,  12},
    {  6,  14,  18,  14,   6},
    {  2,   8,  10,   8,   2},
    {  0,   4,   6,   4,   0},
});

// Sliders want the open middle; the corners are where they see least.
constexpr Table SLIDER_TABLE = table_from({
    {  0,   6,   8,   6,   0},
    {  6,  14,  16,  14,   6},
    {  8,  16,  20,  16,   8},
    {  6,  14,  16,  14,   6},
    {  0,   6,   8,   6,   0},
});

// The king is safer at home and safest out of the centre file, where
// everything points.
constexpr Table KING_TABLE = table_from({
    {-30, -34, -36, -34, -30},
    {-20, -24, -26, -24, -20},
    {-10, -14, -16, -14, -10},
    {  4,   0,  -4,   0,   4},
    { 16,  10,   4,  10,  16},
});

const Table& table_for(PieceType pt) {
    switch (pt) {
        case PAWN:   return PAWN_TABLE;
        case KING:   return KING_TABLE;
        case BISHOP:
        case ROOK:
        case HORSE:
        case DRAGON: return SLIDER_TABLE;
        default:     return GENERAL_TABLE;  // gold, silver, promoted minors
    }
}

int placement(const Position& pos) {
    int score = 0;
    for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        const Table& table = table_for(PieceType(pt));

        Bitboard black = pos.pieces(BLACK, PieceType(pt));
        while (black) score += table[pop_lsb(black)];

        Bitboard white = pos.pieces(WHITE, PieceType(pt));
        while (white) score -= table[mirror(pop_lsb(white))];
    }
    return score;
}

// Shogi kills quickly because a captured piece comes back as a drop, so
// the squares around a king matter more than in chess, and holding
// pieces in hand is itself an attacking asset.
int king_safety_for(const Position& pos, Color us) {
    const Square king = pos.king_square(us);
    const Bitboard ring = STEP_ATTACKS[us][KING][king];

    int attacked = 0, defended = 0;
    Bitboard squares = ring;
    while (squares) {
        const Square s = pop_lsb(squares);
        if (pos.attackers_to(s, ~us)) ++attacked;
        if (pos.attackers_to(s, us)) ++defended;
    }

    int hand_pressure = 0;
    for (int i = 0; i < HAND_TYPE_NB; ++i)
        hand_pressure += pos.hand_count(~us, hand_type(i));

    // A square the enemy attacks and we do not is the one that hurts.
    return -18 * attacked + 6 * defended - 10 * hand_pressure;
}

int material(const Position& pos) {
    int score = 0;
    for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        const int value = PIECE_VALUE[pt];
        if (!value) continue;
        score += value * popcount(pos.pieces(BLACK, PieceType(pt)));
        score -= value * popcount(pos.pieces(WHITE, PieceType(pt)));
    }
    for (int i = 0; i < HAND_TYPE_NB; ++i) {
        const PieceType pt = hand_type(i);
        const int value = PIECE_VALUE[pt] * (100 + HAND_BONUS_PERCENT) / 100;
        score += value * pos.hand_count(BLACK, pt);
        score -= value * pos.hand_count(WHITE, pt);
    }
    return score;
}

}  // namespace

int evaluate(const Position& pos, const EvalWeights& weights) {
    int score = material(pos) * weights.material / 100;

    if (weights.placement) score += placement(pos) * weights.placement / 100;

    if (weights.king_safety) {
        const int safety =
            king_safety_for(pos, BLACK) - king_safety_for(pos, WHITE);
        score += safety * weights.king_safety / 100;
    }

    return pos.side_to_move() == BLACK ? score : -score;
}

}  // namespace pioneer
