// Bounded tsume solver using depth-first proof-number search (df-pn).
//
// The side to move at the root is the attacker. On attacker turns only
// legal checking moves are considered; on defender turns every legal
// evasion is considered. A proof therefore means: "the attacker can
// force checkmate, while checking on every move, within max_plies."
//
// Repetition / GHI policy (v1):
//   - This solver has no transposition table and never shares proof or
//     disproof numbers between paths.
//   - If a key repeats an ancestor within the search, that branch is
//     conservatively treated as not proving checkmate.
//   - The finite ply horizon makes the remaining tree finite.
//
// This is sound but incomplete: transpositions are searched repeatedly,
// and a line that revisits a position before later mating is rejected.
// Position has no pre-root history, so this API is for fresh puzzle
// positions. A caller with game history must not use PROVEN as an
// in-game claim until that history is supplied to the solver.

#pragma once

#include <cstdint>

#include "../core/position.h"

namespace pioneer {

struct MateResult {
    // True only for a completed proof. If false and complete is true,
    // there is no forcing-check mate within max_plies. If both are false,
    // the node budget ended the search and the answer is unknown.
    bool proven = false;
    bool complete = false;
    Move first_move = MOVE_NONE;
    uint64_t nodes = 0;  // tree node records created, including the root
    uint32_t proof = 1;
    uint32_t disproof = 1;
};

// max_plies is root-relative; odd values express the natural tsume
// horizons (attacker moves at plies 1, 3, ...). node_budget bounds tree
// records, not recursive calls. Expansion is atomic: insufficient budget
// yields unknown rather than omitting defenses and risking a false proof.
MateResult solve_mate(const Position& pos, int max_plies,
                      uint64_t node_budget);

}  // namespace pioneer
