#include <string>

#include "../core/bitboard.h"
#include "test.h"

using namespace pioneer;

namespace {

Bitboard bb_of(std::initializer_list<int> squares) {
    Bitboard b = 0;
    for (int s : squares) b |= square_bb(Square(s));
    return b;
}

}  // namespace

TEST(square_usi_round_trip) {
    CHECK_EQ(usi_square(Square(0)), "5a");
    CHECK_EQ(usi_square(Square(24)), "1e");
    CHECK_EQ(usi_square(make_square(3, 0)), "5d");
    for (int s = 0; s < SQUARE_NB; ++s)
        CHECK_EQ(int(parse_usi_square(usi_square(Square(s)))), s);
    CHECK_EQ(int(parse_usi_square("6a")), int(SQ_NONE));
    CHECK_EQ(int(parse_usi_square("5f")), int(SQ_NONE));
}

TEST(move_encoding_round_trip) {
    Move m = make_move(parse_usi_square("5d"), parse_usi_square("5c"));
    CHECK(!is_drop(m) && !is_promotion(m));
    CHECK_EQ(usi_move(m), "5d5c");
    CHECK_EQ(parse_usi_move("5d5c"), m);

    Move p = make_move(parse_usi_square("2b"), parse_usi_square("2a"), true);
    CHECK(is_promotion(p));
    CHECK_EQ(usi_move(p), "2b2a+");
    CHECK_EQ(parse_usi_move("2b2a+"), p);

    Move d = make_drop(SILVER, parse_usi_square("3c"));
    CHECK(is_drop(d));
    CHECK_EQ(int(drop_type(d)), int(SILVER));
    CHECK_EQ(usi_move(d), "S*3c");
    CHECK_EQ(parse_usi_move("S*3c"), d);

    CHECK_EQ(parse_usi_move("X*3c"), MOVE_NONE);
    CHECK_EQ(parse_usi_move("K*3c"), MOVE_NONE);  // king is never in hand
}

TEST(pawn_attacks) {
    // Black pawn on 5d pushes toward rank a.
    CHECK_EQ(STEP_ATTACKS[BLACK][PAWN][make_square(3, 0)],
             square_bb(make_square(2, 0)));
    // White pawn on 1b pushes toward rank e.
    CHECK_EQ(STEP_ATTACKS[WHITE][PAWN][make_square(1, 4)],
             square_bb(make_square(2, 4)));
    // A black pawn on the top rank has nowhere to go (and could never
    // legally stand there unpromoted).
    CHECK_EQ(STEP_ATTACKS[BLACK][PAWN][make_square(0, 2)], Bitboard(0));
}

TEST(silver_attacks_edge_no_wrap) {
    // Black silver on the left edge: forward three plus one back diagonal.
    CHECK_EQ(STEP_ATTACKS[BLACK][SILVER][make_square(2, 0)],
             bb_of({5, 6, 16}));
    // No spill onto col 4 of adjacent ranks.
    CHECK((STEP_ATTACKS[BLACK][SILVER][make_square(2, 0)] &
           (square_bb(Square(9)) | square_bb(Square(19)))) == 0);
}

TEST(gold_attacks) {
    // White gold in the center: forward is rank +1.
    CHECK_EQ(STEP_ATTACKS[WHITE][GOLD][make_square(2, 2)],
             bb_of({16, 17, 18, 11, 13, 7}));
    // Promoted pawn and silver share the gold pattern.
    CHECK_EQ(STEP_ATTACKS[BLACK][PRO_PAWN][12],
             STEP_ATTACKS[BLACK][GOLD][12]);
    CHECK_EQ(STEP_ATTACKS[WHITE][PRO_SILVER][12],
             STEP_ATTACKS[WHITE][GOLD][12]);
}

TEST(king_corner) {
    CHECK_EQ(STEP_ATTACKS[BLACK][KING][0], bb_of({1, 5, 6}));
    CHECK_EQ(popcount(STEP_ATTACKS[WHITE][KING][12]), 8);
}

TEST(rook_attacks_with_blocker) {
    Bitboard occupied = square_bb(Square(13));
    CHECK_EQ(rook_attacks(make_square(2, 2), occupied),
             bb_of({13, 11, 10, 7, 2, 17, 22}));
}

TEST(bishop_attacks_corner) {
    CHECK_EQ(bishop_attacks(make_square(4, 0), 0), bb_of({16, 12, 8, 4}));
}

TEST(horse_dragon_combine_slide_and_steps) {
    CHECK_EQ(attacks_from(BLACK, HORSE, Square(12), 0),
             bishop_attacks(Square(12), 0) | bb_of({7, 11, 13, 17}));
    CHECK_EQ(attacks_from(WHITE, DRAGON, Square(12), 0),
             rook_attacks(Square(12), 0) | bb_of({6, 8, 16, 18}));
    // Blockers cut the sliding component but not the king steps.
    Bitboard occupied = bb_of({8, 16});
    CHECK_EQ(attacks_from(BLACK, HORSE, Square(12), occupied),
             bb_of({8, 16, 6, 0, 18, 24, 7, 11, 13, 17}));
}

TEST(file_masks) {
    CHECK_EQ(FILE_BB[0], bb_of({0, 5, 10, 15, 20}));
    CHECK_EQ(FILE_BB[4], bb_of({4, 9, 14, 19, 24}));
}

TEST(ray_next_edges) {
    CHECK_EQ(int(RAY_NEXT[0][0]), int(SQ_NONE));   // N from rank a
    CHECK_EQ(int(RAY_NEXT[2][4]), int(SQ_NONE));   // E from col 4
    CHECK_EQ(int(RAY_NEXT[4][22]), int(SQ_NONE));  // S from rank e
    CHECK_EQ(int(RAY_NEXT[0][22]), 17);
}

TEST(between_bb_strict_interior) {
    // Orthogonal and diagonal lines contain only their interior squares.
    CHECK_EQ(BETWEEN_BB[0][20], bb_of({5, 10, 15}));
    CHECK_EQ(BETWEEN_BB[10][14], bb_of({11, 12, 13}));
    CHECK_EQ(BETWEEN_BB[0][24], bb_of({6, 12, 18}));
    CHECK_EQ(BETWEEN_BB[4][20], bb_of({8, 12, 16}));

    // The table is symmetric for every square pair.
    for (int from = 0; from < SQUARE_NB; ++from)
        for (int to = 0; to < SQUARE_NB; ++to)
            CHECK_EQ(BETWEEN_BB[from][to], BETWEEN_BB[to][from]);

    // Adjacent aligned squares and non-aligned squares have no interior.
    CHECK_EQ(BETWEEN_BB[0][1], Bitboard(0));
    CHECK_EQ(BETWEEN_BB[0][5], Bitboard(0));
    CHECK_EQ(BETWEEN_BB[0][6], Bitboard(0));
    CHECK_EQ(BETWEEN_BB[0][7], Bitboard(0));
    CHECK_EQ(BETWEEN_BB[12][12], Bitboard(0));

    // Neither endpoint is ever part of the strict-between mask.
    CHECK((BETWEEN_BB[0][20] & (square_bb(0) | square_bb(20))) == 0);
    CHECK((BETWEEN_BB[24][0] & (square_bb(24) | square_bb(0))) == 0);
}
