// Bitboards for the 5x5 board: one uint32_t, bit i = square i.
// All attack tables are built at compile time.

#pragma once

#include <array>
#include <initializer_list>

#include "types.h"

namespace pioneer {

using Bitboard = uint32_t;
constexpr Bitboard BOARD_BB = (1u << SQUARE_NB) - 1;

constexpr Bitboard square_bb(Square s) { return 1u << s; }

inline int popcount(Bitboard b) { return __builtin_popcount(b); }
inline Square lsb(Bitboard b) {
    assert(b);
    return Square(__builtin_ctz(b));
}
inline Square pop_lsb(Bitboard& b) {
    Square s = lsb(b);
    b &= b - 1;
    return s;
}

struct Delta { int dr, dc; };

// Direction indices: N NE E SE S SW W NW, where N points toward rank 0
// (the direction Black moves).
constexpr Delta DIR_DELTA[8] = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}};
constexpr int ORTH_DIRS[4] = {0, 2, 4, 6};
constexpr int DIAG_DIRS[4] = {1, 3, 5, 7};

// RAY_NEXT[dir][sq]: the next square along dir, or SQ_NONE at the edge.
constexpr auto build_ray_next() {
    std::array<std::array<int8_t, SQUARE_NB>, 8> t{};
    for (int d = 0; d < 8; ++d)
        for (int s = 0; s < SQUARE_NB; ++s) {
            int r = s / 5 + DIR_DELTA[d].dr;
            int c = s % 5 + DIR_DELTA[d].dc;
            t[d][s] = is_square(r, c) ? int8_t(r * 5 + c) : int8_t(SQ_NONE);
        }
    return t;
}
inline constexpr auto RAY_NEXT = build_ray_next();

// STEP_ATTACKS[color][piece type][sq]: attack set of the non-sliding
// component. Bishop/rook have none; horse/dragon hold only their extra
// king steps - their sliding component comes from the ray walkers below.
constexpr auto build_step_attacks() {
    std::array<std::array<std::array<Bitboard, SQUARE_NB>, PIECE_TYPE_NB>,
               COLOR_NB> t{};
    for (int c = 0; c < COLOR_NB; ++c) {
        const int f = (c == BLACK) ? -1 : 1;  // forward rank delta
        for (int s = 0; s < SQUARE_NB; ++s) {
            auto set = [&](PieceType pt, std::initializer_list<Delta> ds) {
                Bitboard b = 0;
                for (Delta d : ds) {
                    int r = s / 5 + d.dr, cc = s % 5 + d.dc;
                    if (is_square(r, cc)) b |= 1u << (r * 5 + cc);
                }
                t[c][pt][s] = b;
            };
            set(PAWN,   {{f, 0}});
            set(SILVER, {{f, -1}, {f, 0}, {f, 1}, {-f, -1}, {-f, 1}});
            set(GOLD,   {{f, -1}, {f, 0}, {f, 1}, {0, -1}, {0, 1}, {-f, 0}});
            set(KING,   {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                         {0, 1}, {1, -1}, {1, 0}, {1, 1}});
            set(HORSE,  {{-1, 0}, {0, -1}, {0, 1}, {1, 0}});
            set(DRAGON, {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
            t[c][PRO_PAWN][s] = t[c][PRO_SILVER][s] = t[c][GOLD][s];
        }
    }
    return t;
}
inline constexpr auto STEP_ATTACKS = build_step_attacks();

// Column masks (nifu checks).
constexpr auto build_file_bb() {
    std::array<Bitboard, FILE_NB> t{};
    for (int c = 0; c < FILE_NB; ++c)
        for (int r = 0; r < RANK_NB; ++r) t[c] |= 1u << (r * 5 + c);
    return t;
}
inline constexpr auto FILE_BB = build_file_bb();

inline Bitboard slider_attacks(Square s, Bitboard occupied, const int dirs[4]) {
    Bitboard b = 0;
    for (int i = 0; i < 4; ++i)
        for (int8_t n = RAY_NEXT[dirs[i]][s]; n != SQ_NONE;
             n = RAY_NEXT[dirs[i]][n]) {
            b |= square_bb(n);
            if (occupied & square_bb(n)) break;
        }
    return b;
}
inline Bitboard bishop_attacks(Square s, Bitboard occupied) {
    return slider_attacks(s, occupied, DIAG_DIRS);
}
inline Bitboard rook_attacks(Square s, Bitboard occupied) {
    return slider_attacks(s, occupied, ORTH_DIRS);
}

// Full attack set of a piece standing on s.
inline Bitboard attacks_from(Color c, PieceType pt, Square s,
                             Bitboard occupied) {
    switch (pt) {
        case BISHOP: return bishop_attacks(s, occupied);
        case ROOK:   return rook_attacks(s, occupied);
        case HORSE:  return bishop_attacks(s, occupied) | STEP_ATTACKS[c][HORSE][s];
        case DRAGON: return rook_attacks(s, occupied) | STEP_ATTACKS[c][DRAGON][s];
        default:     return STEP_ATTACKS[c][pt][s];
    }
}

}  // namespace pioneer
