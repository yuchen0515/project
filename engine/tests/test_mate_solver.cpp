#include <algorithm>
#include <string>
#include <vector>

#include "../agents/mate_solver.h"
#include "../core/movegen.h"
#include "alphabeta_test_access.h"
#include "test.h"

using namespace pioneer;

namespace {

struct Puzzle {
    const char* name;
    const char* sfen;
    int plies;
    const char* first_move;
    bool mate;
};

constexpr Puzzle PUZZLES[] = {
    {"mate-1a", "k2r1/5/1B3/5/4K b G 1", 1, "G*5b", true},
    {"mate-1b", "4k/2S1p/1B3/K4/5 b - 1", 1, "3b2a+", true},
    {"mate-1c", "1k3/2r2/5/KG+r2/5 w - 1", 1, "3b5b", true},
    {"mate-3a", "2r1k/2BP1/5/B4/1K3 b - 1", 3, "3b2a+", true},
    {"mate-3b", "1kb2/2r2/5/KG+r2/5 w B 1", 3, "3b5b", true},
    {"mate-3c", "k4/5/5/K1s2/2B2 w rg 1", 3, "G*4c", true},
    {"mate-5a", "5/2k2/4R/1K3/1G3 b G 1", 5, "1c1b", true},
    {"mate-5b", "1k3/2r2/5/bG+r2/K2s1 w P 1", 5, "3d4d", true},
    {"mate-7a", "1g3/k4/5/SBK2/1r3 b - 1", 7, "5d4c", true},
    {"escape", "k4/4g/5/1R3/4K b - 1", 7, "none", false},
    {"interpose", "kp3/1p3/5/5/1R2K b s 1", 7, "none", false},
    {"capture", "k4/5/5/5/4K b G 1", 7, "none", false},
    {"pawn-drop", "3pk/5/4G/5/K4 b Pr 1", 7, "none", false},
};

bool repeats(const std::vector<uint64_t>& path, uint64_t key) {
    return std::find(path.begin(), path.end(), key) != path.end();
}

// Frozen independent oracle: plain bounded OR/AND recursion, with no
// proof numbers and no call into mate_solver. It intentionally uses only
// public rules APIs.
bool reference_mate(const Position& pos, Color attacker, int remaining,
                    std::vector<uint64_t>& path, uint64_t& nodes) {
    ++nodes;
    const bool attacker_turn = pos.side_to_move() == attacker;
    MoveList legal;
    generate_legal(pos, legal);

    if (!attacker_turn) {
        if (!pos.in_check()) return false;
        if (legal.empty()) return true;
        if (remaining <= 0) return false;

        for (Move move : legal) {
            Position next = pos;
            next.apply(move);
            if (repeats(path, next.key())) return false;
            path.push_back(next.key());
            const bool still_mates =
                reference_mate(next, attacker, remaining - 1, path, nodes);
            path.pop_back();
            if (!still_mates) return false;
        }
        return true;
    }

    if (remaining <= 0) return false;
    for (Move move : legal) {
        if (!gives_check(pos, move)) continue;
        Position next = pos;
        next.apply(move);
        if (repeats(path, next.key())) continue;
        path.push_back(next.key());
        const bool mates =
            reference_mate(next, attacker, remaining - 1, path, nodes);
        path.pop_back();
        if (mates) return true;
    }
    return false;
}

std::vector<Move> reference_winning_moves(const Position& pos, int max_plies,
                                          uint64_t& nodes) {
    std::vector<Move> winning;
    if (max_plies <= 0) return winning;
    MoveList legal;
    generate_legal(pos, legal);
    std::vector<uint64_t> path{pos.key()};

    for (Move move : legal) {
        if (!gives_check(pos, move)) continue;
        Position next = pos;
        next.apply(move);
        if (repeats(path, next.key())) continue;
        path.push_back(next.key());
        if (reference_mate(next, pos.side_to_move(), max_plies - 1,
                           path, nodes))
            winning.push_back(move);
        path.pop_back();
    }
    return winning;
}

struct AlphaProof {
    bool mate;
    int distance;
    uint64_t nodes;
};

AlphaProof alpha_proof(const Position& pos, int max_plies,
                       bool search_pruning = true) {
    AlphaBetaAgent::Options options;
    options.depth = std::max(1, max_plies);
    options.quiescence = false;
    options.transposition = false;
    options.move_ordering = false;
    options.null_move = search_pruning;
    options.late_move_reductions = search_pruning;
    AlphaBetaAgent oracle(options, 1);

    const int score =
        AlphaBetaAgentTestAccess::exact_score(oracle, pos, max_plies);
    const bool mate = score >= MATE_THRESHOLD;
    return {mate, mate ? MATE_SCORE - score : -1, oracle.nodes()};
}

Position position_of(const char* sfen) {
    Position pos;
    CHECK(pos.set(sfen));
    return pos;
}

MoveList replies_after(const Position& pos, const char* usi) {
    const Move move = parse_usi_move(usi);
    CHECK(is_legal(pos, move));
    CHECK(gives_check(pos, move));
    Position next = pos;
    next.apply(move);
    MoveList replies;
    generate_legal(next, replies);
    return replies;
}

}  // namespace

TEST(dfpn_tsume_suite_agrees_with_reference_and_alphabeta) {
    for (const Puzzle& puzzle : PUZZLES) {
        const Position pos = position_of(puzzle.sfen);
        CHECK(!pos.in_check());

        const MateResult result =
            solve_mate(pos, puzzle.plies, 1000000);
        CHECK(result.complete);
        CHECK_EQ(result.proven, puzzle.mate);

        uint64_t reference_nodes = 0;
        const std::vector<Move> reference =
            reference_winning_moves(pos, puzzle.plies, reference_nodes);
        CHECK_EQ(!reference.empty(), puzzle.mate);

        const AlphaProof pruned = alpha_proof(pos, puzzle.plies);
        const AlphaProof unpruned =
            alpha_proof(pos, puzzle.plies, false);
        CHECK_EQ(pruned.mate, puzzle.mate);
        CHECK_EQ(unpruned.mate, puzzle.mate);

        if (puzzle.mate) {
            const Move expected = parse_usi_move(puzzle.first_move);
            CHECK_EQ(result.first_move, expected);
            CHECK_EQ(reference.size(), size_t(1));
            CHECK_EQ(reference[0], expected);
            CHECK_EQ(pruned.distance, puzzle.plies);
            CHECK_EQ(unpruned.distance, puzzle.plies);

            // The advertised length is minimal, not merely sufficient.
            const int shorter = std::max(0, puzzle.plies - 2);
            const MateResult too_short =
                solve_mate(pos, shorter, 1000000);
            CHECK(too_short.complete);
            CHECK(!too_short.proven);
            uint64_t short_nodes = 0;
            CHECK(reference_winning_moves(pos, shorter, short_nodes).empty());
            CHECK(!alpha_proof(pos, shorter).mate);
            CHECK(!alpha_proof(pos, shorter, false).mate);
        } else {
            CHECK_EQ(result.first_move, MOVE_NONE);
            CHECK_EQ(pruned.distance, -1);
            CHECK_EQ(unpruned.distance, -1);
        }
    }
}

TEST(dfpn_negative_fixtures_exercise_distinct_defenses) {
    const MoveList escapes =
        replies_after(position_of("k4/4g/5/1R3/4K b - 1"), "4d5d");
    CHECK_EQ(escapes.size(), 2);
    CHECK(escapes.contains(parse_usi_move("5a4a")));
    CHECK(escapes.contains(parse_usi_move("5a4b")));

    const MoveList interpositions =
        replies_after(position_of("kp3/1p3/5/5/1R2K b s 1"), "4e5e");
    CHECK_EQ(interpositions.size(), 3);
    CHECK(interpositions.contains(parse_usi_move("S*5b")));
    CHECK(interpositions.contains(parse_usi_move("S*5c")));
    CHECK(interpositions.contains(parse_usi_move("S*5d")));

    const MoveList captures =
        replies_after(position_of("k4/5/5/5/4K b G 1"), "G*5b");
    CHECK_EQ(captures.size(), 1);
    CHECK(captures.contains(parse_usi_move("5a5b")));
}

TEST(dfpn_rejects_illegal_pawn_drop_mate) {
    const Position pos = position_of("3pk/5/4G/5/K4 b Pr 1");
    const Move pawn_mate = parse_usi_move("P*1b");

    MoveList pseudo, legal;
    generate_pseudo_legal(pos, pseudo);
    generate_legal(pos, legal);
    CHECK(pseudo.contains(pawn_mate));
    CHECK(!legal.contains(pawn_mate));
    CHECK(!is_legal(pos, pawn_mate));

    Position after = pos;
    after.apply(pawn_mate);
    CHECK(is_checkmate(after));

    int pseudo_mates = 0;
    Move only_pseudo_mate = MOVE_NONE;
    for (Move move : pseudo) {
        Position next = pos;
        next.apply(move);
        if (is_checkmate(next)) {
            ++pseudo_mates;
            only_pseudo_mate = move;
        }
    }
    CHECK_EQ(pseudo_mates, 1);
    CHECK_EQ(only_pseudo_mate, pawn_mate);

    const MateResult result = solve_mate(pos, 7, 1000000);
    CHECK(result.complete);
    CHECK(!result.proven);
}

TEST(dfpn_budget_exhaustion_is_unknown_not_a_partial_proof) {
    const Position pos =
        position_of("5/2k2/4R/1K3/1G3 b G 1");
    const MateResult result = solve_mate(pos, 5, 1);
    CHECK(!result.proven);
    CHECK(!result.complete);
    CHECK_EQ(result.first_move, MOVE_NONE);
    CHECK_EQ(result.nodes, uint64_t(1));
}
