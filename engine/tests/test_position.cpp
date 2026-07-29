#include <array>

#include "../core/position.h"
#include "test.h"

using namespace pioneer;

namespace {

Position from(const std::string& sfen) {
    Position pos;
    CHECK(pos.set(sfen));
    return pos;
}

}  // namespace

TEST(start_position_layout) {
    Position pos;
    CHECK(pos.is_consistent());
    CHECK_EQ(int(pos.side_to_move()), int(BLACK));
    CHECK_EQ(pos.game_ply(), 1);

    // Black king on 5e, rook on 1e; White king on 1a, rook on 5a.
    CHECK_EQ(int(pos.king_square(BLACK)), int(parse_usi_square("5e")));
    CHECK_EQ(int(pos.king_square(WHITE)), int(parse_usi_square("1a")));
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("1e")))), int(ROOK));
    CHECK_EQ(int(color_of(pos.piece_on(parse_usi_square("5a")))), int(WHITE));
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("5d")))), int(PAWN));
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("1b")))), int(PAWN));

    CHECK_EQ(popcount(pos.pieces(BLACK)), 6);
    CHECK_EQ(popcount(pos.pieces(WHITE)), 6);
    CHECK(pos.hand_empty(BLACK) && pos.hand_empty(WHITE));
    CHECK(!pos.in_check());
}

TEST(sfen_round_trip) {
    CHECK_EQ(Position().sfen(), std::string(START_SFEN));

    const char* cases[] = {
        "rbsgk/4p/5/P4/KGSBR b - 1",
        "4k/5/5/5/K4 w - 12",
        "+r3k/5/5/5/K3+b b 2Pgs 7",
        "k4/5/2+P2/5/4K w RBGSrbgs 40",
    };
    for (const char* s : cases) {
        Position pos = from(s);
        CHECK(pos.is_consistent());
        CHECK_EQ(pos.sfen(), std::string(s));
    }
}

TEST(sfen_rejects_malformed) {
    Position pos;
    const char* bad[] = {
        "rbsgk/4p/5/P4 b - 1",        // too few ranks
        "rbsgk/4p/5/P4/KGSBRR b - 1", // rank too wide
        "rbsgk/4p/6/P4/KGSBR b - 1",  // run longer than the board
        "rbsgk/4p/5/P4/KGSBR x - 1",  // bad side letter
        "rbsgk/4p/5/P4/KGSBR b K 1",  // king can never be in hand
        "rbsgk/4p/5/P4/KGSBR b 3P 1", // more copies than exist
        "rbsg1/4p/5/P4/KGSBR b - 1",  // white king missing
        "+k3K/5/5/5/5 b - 1",         // king cannot be promoted
        "rbsgk/4p/5/P4/KGSBR b - 0",  // ply is 1-based
        "R3k/5/5/5/K4 b - 1",         // white king already capturable
        "4k/5/5/5/K3r w - 1",         // same, colors reversed
        "r3k/5/5/5/K3R b Rr 1",       // four rooks; only two exist
        "4k/5/2+P2/5/K4 b 2P 1",      // three pawns counting the promoted one
        "k3k/5/5/5/K4 b - 1",         // two white kings
    };
    for (const char* s : bad) CHECK(!pos.set(s));
}

TEST(apply_quiet_move) {
    Position pos;
    const uint64_t before = pos.key();
    pos.apply(parse_usi_move("5d5c"));  // black pawn push

    CHECK(pos.is_consistent());
    CHECK_EQ(int(pos.side_to_move()), int(WHITE));
    CHECK_EQ(pos.game_ply(), 2);
    CHECK(pos.key() != before);
    CHECK_EQ(int(pos.piece_on(parse_usi_square("5d"))), int(NO_PIECE));
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("5c")))), int(PAWN));
    CHECK_EQ(pos.sfen(), std::string("rbsgk/4p/P4/5/KGSBR w - 2"));
}

TEST(apply_null_only_toggles_side_ply_and_side_hash) {
    Position pos = from("4k/5/2+P2/5/K4 b Rbgs 7");
    const uint64_t original_key = pos.key();

    std::array<Piece, SQUARE_NB> original_board{};
    for (int square = 0; square < SQUARE_NB; ++square)
        original_board[square] = pos.piece_on(Square(square));

    std::array<std::array<int, HAND_TYPE_NB>, COLOR_NB> original_hands{};
    for (int color = 0; color < COLOR_NB; ++color)
        for (int type = 0; type < HAND_TYPE_NB; ++type)
            original_hands[color][type] =
                pos.hand_count(Color(color), hand_type(type));

    auto check_material_unchanged = [&] {
        for (int square = 0; square < SQUARE_NB; ++square)
            CHECK_EQ(int(pos.piece_on(Square(square))),
                     int(original_board[square]));
        for (int color = 0; color < COLOR_NB; ++color)
            for (int type = 0; type < HAND_TYPE_NB; ++type)
                CHECK_EQ(pos.hand_count(Color(color), hand_type(type)),
                         original_hands[color][type]);
    };

    pos.apply_null();
    CHECK(pos.is_consistent());
    check_material_unchanged();
    CHECK_EQ(int(pos.side_to_move()), int(WHITE));
    CHECK_EQ(pos.game_ply(), 8);
    CHECK_EQ(pos.key(), original_key ^ zobrist::KEYS.side);
    CHECK_EQ(pos.sfen(),
             std::string("4k/5/2+P2/5/K4 w Rbgs 8"));

    pos.apply_null();
    CHECK(pos.is_consistent());
    check_material_unchanged();
    CHECK_EQ(int(pos.side_to_move()), int(BLACK));
    CHECK_EQ(pos.game_ply(), 9);
    CHECK_EQ(pos.key(), original_key);
    CHECK_EQ(pos.sfen(),
             std::string("4k/5/2+P2/5/K4 b Rbgs 9"));
}

TEST(capture_moves_piece_to_hand_unpromoted) {
    // Black dragon on 5b captures the white promoted pawn on 5a; a
    // promoted piece reverts to its base type when captured.
    Position pos = from("+p3k/+R4/5/5/K4 b - 1");
    pos.apply(parse_usi_move("5b5a"));

    CHECK(pos.is_consistent());
    CHECK_EQ(pos.hand_count(BLACK, PAWN), 1);
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("5a")))), int(DRAGON));
    CHECK_EQ(pos.sfen(), std::string("+R3k/5/5/5/K4 w P 2"));
}

TEST(promotion_on_reaching_far_rank) {
    Position pos = from("4k/P4/5/5/K4 b - 1");
    pos.apply(parse_usi_move("5b5a+"));

    CHECK(pos.is_consistent());
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("5a")))), int(PRO_PAWN));
    CHECK_EQ(pos.sfen(), std::string("+P3k/5/5/5/K4 w - 2"));
}

TEST(drop_removes_from_hand) {
    Position pos = from("4k/5/5/5/K4 b P 1");
    pos.apply(parse_usi_move("P*3c"));

    CHECK(pos.is_consistent());
    CHECK_EQ(pos.hand_count(BLACK, PAWN), 0);
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("3c")))), int(PAWN));
    CHECK_EQ(pos.sfen(), std::string("4k/5/2P2/5/K4 w - 2"));
}

TEST(zobrist_key_is_incremental_and_path_independent) {
    Position pos;
    for (const char* m : {"5d5c", "1b1c", "5e5d", "1a1b"}) {
        pos.apply(parse_usi_move(m));
        CHECK(pos.is_consistent());  // includes key_ == recomputed key
    }

    // Transposition: both kings walk to the same squares by different
    // routes, so the resulting positions must hash equal.
    Position route_a = from("4k/5/5/5/K4 b - 1");
    for (const char* m : {"5e4e", "1a2a", "4e4d", "2a2b"})
        route_a.apply(parse_usi_move(m));
    Position route_b = from("4k/5/5/5/K4 b - 1");
    for (const char* m : {"5e5d", "1a1b", "5d4d", "1b2b"})
        route_b.apply(parse_usi_move(m));
    CHECK_EQ(route_a.sfen(), route_b.sfen());
    CHECK_EQ(route_a.key(), route_b.key());

    // A position differing only in side to move must hash differently.
    Position black_to_move = from("4k/5/5/5/K4 b - 1");
    Position white_to_move = from("4k/5/5/5/K4 w - 1");
    CHECK(black_to_move.key() != white_to_move.key());

    // Hand contents are part of the key.
    Position empty_hand = from("4k/5/5/5/K4 b - 1");
    Position one_pawn = from("4k/5/5/5/K4 b P 1");
    Position two_pawns = from("4k/5/5/5/K4 b 2P 1");
    CHECK(empty_hand.key() != one_pawn.key());
    CHECK(one_pawn.key() != two_pawns.key());
}

TEST(attackers_to_and_check_detection) {
    // White rook on 5a bears down the file on the black king on 5e.
    Position pos = from("r3k/5/5/5/K4 b - 1");
    CHECK(pos.in_check());
    CHECK_EQ(pos.attackers_to(pos.king_square(BLACK), WHITE),
             square_bb(parse_usi_square("5a")));

    // Interpose a piece and the check is gone.
    Position blocked = from("r3k/5/2G2/5/K4 b - 1");
    CHECK(blocked.in_check());
    Position blocked2 = from("r3k/G4/5/5/K4 b - 1");
    CHECK(!blocked2.in_check());

    // Pawns attack forward only: a white pawn on 5d checks a black king
    // on 5e, a black pawn there does not.
    CHECK(from("4k/5/5/p4/K4 b - 1").in_check());
    CHECK(!from("4k/5/5/P4/K4 b - 1").in_check());

    // A horse attacks both along its diagonals and by king step.
    Position horse = from("4k/5/5/5/K3+B w - 1");
    const Square horse_sq = parse_usi_square("1e");
    CHECK_EQ(horse.attackers_to(parse_usi_square("1d"), BLACK),
             square_bb(horse_sq));  // king step
    CHECK_EQ(horse.attackers_to(parse_usi_square("3c"), BLACK),
             square_bb(horse_sq));  // diagonal
    CHECK_EQ(horse.attackers_to(parse_usi_square("3d"), BLACK), Bitboard(0));
}

TEST(attackers_to_with_hypothetical_occupancy) {
    const Square black_king = parse_usi_square("5e");
    const Square white_rook = parse_usi_square("5a");
    const Square rook_blocker = parse_usi_square("5b");

    Position rook_xray = from("r3k/G4/5/5/K4 b - 1");
    CHECK_EQ(rook_xray.attackers_to(black_king, WHITE), Bitboard(0));
    const Bitboard rook_blocker_removed =
        rook_xray.occupied() & ~square_bb(rook_blocker);
    CHECK_EQ(rook_xray.attackers_to(black_king, WHITE,
                                   rook_blocker_removed),
             square_bb(white_rook));

    Position open_rook_file = from("r3k/5/5/5/K4 b - 1");
    CHECK_EQ(open_rook_file.attackers_to(black_king, WHITE),
             square_bb(white_rook));
    const Bitboard artificial_blocker =
        open_rook_file.occupied() | square_bb(parse_usi_square("5c"));
    CHECK_EQ(open_rook_file.attackers_to(black_king, WHITE,
                                        artificial_blocker),
             Bitboard(0));

    const Square white_bishop = parse_usi_square("1a");
    const Square bishop_blocker = parse_usi_square("3c");
    Position bishop_xray = from("k3b/5/2G2/5/K4 b - 1");
    CHECK_EQ(bishop_xray.attackers_to(black_king, WHITE), Bitboard(0));
    const Bitboard bishop_blocker_removed =
        bishop_xray.occupied() & ~square_bb(bishop_blocker);
    CHECK_EQ(bishop_xray.attackers_to(black_king, WHITE,
                                     bishop_blocker_removed),
             square_bb(white_bishop));
}
