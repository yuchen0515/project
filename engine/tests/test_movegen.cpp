#include <set>
#include <string>

#include "../core/movegen.h"
#include "test.h"

using namespace pioneer;

namespace {

Position from(const std::string& sfen) {
    Position pos;
    CHECK(pos.set(sfen));
    return pos;
}

// Sorted and space-joined so a mismatch reads as a diff of two lines.
std::string legal_moves(const Position& pos) {
    MoveList list;
    generate_legal(pos, list);
    std::set<std::string> sorted;
    for (Move m : list) sorted.insert(usi_move(m));

    std::string out;
    for (const std::string& m : sorted) {
        if (!out.empty()) out += ' ';
        out += m;
    }
    return out;
}

bool has(const Position& pos, const char* usi) {
    MoveList list;
    generate_legal(pos, list);
    return list.contains(parse_usi_move(usi));
}

int count_legal(const Position& pos) {
    MoveList list;
    generate_legal(pos, list);
    return list.size();
}

}  // namespace

TEST(start_position_move_list) {
    // Hand-enumerated from the start position: rook 3 (1e1b takes the
    // white pawn), bishop 4, silver 3, gold 2, pawn 1, king 1.
    CHECK_EQ(legal_moves(Position()),
             std::string("1e1b 1e1c 1e1d 2e1d 2e3d 2e4c 2e5b 3e2d 3e3d 3e4d "
                         "4e3d 4e4d 5d5c 5e4d"));
}

TEST(promotion_is_optional_except_for_pawns) {
    // Black silver stepping into the far rank may stay unpromoted.
    Position silver = from("4k/S4/5/5/K4 b - 1");
    CHECK(has(silver, "5b5a"));
    CHECK(has(silver, "5b5a+"));

    // A pawn reaching the far rank has no choice.
    Position pawn = from("4k/P4/5/5/K4 b - 1");
    CHECK(!has(pawn, "5b5a"));
    CHECK(has(pawn, "5b5a+"));

    // Leaving the zone also allows promotion.
    Position rook = from("R4/4k/5/5/K4 b - 1");
    CHECK(has(rook, "5a5b"));
    CHECK(has(rook, "5a5b+"));

    // Gold and king never promote.
    Position gold = from("4k/G4/5/5/K4 b - 1");
    CHECK(has(gold, "5b5a"));
    CHECK(!has(gold, "5b5a+"));
}

TEST(nifu_bans_a_second_pawn_on_the_file) {
    Position pos = from("4k/5/P4/5/K4 b P 1");
    CHECK(!has(pos, "P*5b"));  // same file as the pawn on 5c
    CHECK(!has(pos, "P*5d"));
    CHECK(has(pos, "P*4c"));   // neighbouring file is fine

    // A promoted pawn does not occupy the file for nifu purposes.
    Position promoted = from("4k/5/+P4/5/K4 b P 1");
    CHECK(has(promoted, "P*5b"));
}

TEST(pawn_drop_banned_on_the_last_rank) {
    Position black = from("4k/5/5/5/K4 b P 1");
    CHECK(!has(black, "P*4a"));
    CHECK(has(black, "P*4b"));

    Position white = from("4k/5/5/5/K4 w p 1");
    CHECK(!has(white, "P*4e"));
    CHECK(has(white, "P*4d"));

    // Other pieces have no such restriction.
    Position silver = from("4k/5/5/5/K4 b S 1");
    CHECK(has(silver, "S*4a"));
}

TEST(uchifuzume_pawn_drop_mate_is_illegal) {
    // White king on 1a, hemmed in by its own pawn on 2a; a black gold
    // on 1c covers 2b and defends 1b. Dropping a pawn on 1b is mate,
    // and mate by pawn drop is forbidden.
    Position pos = from("3pk/5/4G/5/K4 b PG 1");
    CHECK(is_checkmate(from("3pk/4P/4G/5/K4 w G 1")));
    CHECK(!has(pos, "P*1b"));

    // The same mate delivered by a gold drop is perfectly legal.
    CHECK(has(pos, "G*1b"));

    // A pawn drop that only gives check, not mate, is legal too: here
    // the white king can capture the pawn.
    Position check_only = from("4k/5/5/5/K4 b P 1");
    CHECK(has(check_only, "P*1b"));
}

TEST(king_may_not_move_into_check_or_stay_pinned) {
    // White rook on 5a rakes the file; the black king must leave it.
    Position pos = from("r3k/5/5/5/K4 b - 1");
    CHECK(pos.in_check());
    CHECK(!has(pos, "5e5d"));  // still on the rook's file
    CHECK_EQ(legal_moves(pos), std::string("5e4d 5e4e"));

    // A piece shielding the king cannot step aside.
    Position pinned = from("r3k/5/5/G4/K4 b - 1");
    CHECK(!has(pinned, "5d4c"));
    CHECK(has(pinned, "5d5c"));  // moving along the pin stays legal
}

TEST(checkmate_and_stalemate_detection) {
    // Gold on 5b mates the cornered king: it covers both flight squares
    // and the rook behind it makes the gold untouchable.
    Position mate = from("k4/G4/5/5/R3K w - 1");
    CHECK(mate.in_check());
    CHECK(is_checkmate(mate));
    CHECK_EQ(count_legal(mate), 0);

    // Without the rook the king simply takes the gold.
    Position not_mate = from("k4/G4/5/5/4K w - 1");
    CHECK(not_mate.in_check());
    CHECK(!is_checkmate(not_mate));
    CHECK(has(not_mate, "5a5b"));

    // Every flight square covered but the king itself unattacked:
    // stalemate, which shogi also scores as a loss - callers tell the
    // two apart by asking whether the king was in check.
    Position stale = from("k4/4R/5/5/1R2K w - 1");
    CHECK(!stale.in_check());
    CHECK(!has_legal_move(stale));
    CHECK(!is_checkmate(stale));
}

TEST(capture_and_drop_interaction) {
    // Capturing a promoted rook puts an unpromoted rook in hand, which
    // can then be dropped anywhere empty.
    Position pos = from("4k/5/4+r/5/K1B2 b - 1");
    pos.apply(parse_usi_move("3e1c"));
    CHECK_EQ(pos.hand_count(BLACK, ROOK), 1);
    CHECK_EQ(int(type_of(pos.piece_on(parse_usi_square("1c")))), int(BISHOP));

    // A rook in hand drops on any empty square - no restrictions apply
    // to anything but pawns.
    Position with_rook = from("4k/5/5/5/K4 b R 1");
    MoveList list;
    generate_legal(with_rook, list);
    int rook_drops = 0;
    for (Move m : list)
        if (is_drop(m) && drop_type(m) == ROOK) ++rook_drops;
    CHECK_EQ(rook_drops, popcount(~with_rook.occupied() & BOARD_BB));
}

TEST(every_generated_move_keeps_the_position_consistent) {
    Position pos;
    MoveList list;
    generate_legal(pos, list);
    for (Move m : list) {
        Position next = pos;
        next.apply(m);
        CHECK(next.is_consistent());
        CHECK(!next.attackers_to(next.king_square(BLACK), WHITE));
    }
}
