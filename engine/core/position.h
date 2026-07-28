// Position: board, hands, side to move, zobrist key, SFEN I/O.
//
// Positions are meant to be copied (copy-make): the whole object is a
// few dozen bytes, so search code takes a copy and calls apply() on it
// rather than doing undo bookkeeping.

#pragma once

#include <array>
#include <string>

#include "bitboard.h"
#include "types.h"

namespace pioneer {

// Standard minishogi start position.
constexpr const char* START_SFEN = "rbsgk/4p/5/P4/KGSBR b - 1";

namespace zobrist {

constexpr uint64_t splitmix64_next(uint64_t& state) {
    state += 0x9E3779B97F4A7C15ull;
    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// Generation order is part of the format: reordering breaks any stored
// hashes. Seed chain: PSQ tables, then HAND tables, then SIDE.
struct Keys {
    uint64_t psq[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
    // Hand contribution is XOR of hand[c][t][k] for k = 1..count.
    uint64_t hand[COLOR_NB][HAND_TYPE_NB][3];
    uint64_t side;
};

constexpr Keys build_keys() {
    Keys k{};
    uint64_t state = 0x50696F6E65657232ull;  // "Pioneer2"
    for (int c = 0; c < COLOR_NB; ++c)
        for (int pt = 0; pt < PIECE_TYPE_NB; ++pt)
            for (int s = 0; s < SQUARE_NB; ++s)
                k.psq[c][pt][s] = splitmix64_next(state);
    for (int c = 0; c < COLOR_NB; ++c)
        for (int t = 0; t < HAND_TYPE_NB; ++t) {
            k.hand[c][t][0] = 0;
            for (int n = 1; n < 3; ++n) k.hand[c][t][n] = splitmix64_next(state);
        }
    k.side = splitmix64_next(state);
    return k;
}

inline constexpr Keys KEYS = build_keys();

}  // namespace zobrist

class Position {
   public:
    Position() { set(START_SFEN); }

    // Returns false (leaving the position unspecified) on malformed input.
    bool set(const std::string& sfen);
    std::string sfen() const;

    Color side_to_move() const { return side_; }
    int game_ply() const { return ply_; }
    uint64_t key() const { return key_; }

    Piece piece_on(Square s) const { return board_[s]; }
    int hand_count(Color c, PieceType pt) const {
        return hands_[c][hand_index(pt)];
    }
    bool hand_empty(Color c) const;

    Bitboard pieces(Color c) const { return by_color_[c]; }
    Bitboard pieces(Color c, PieceType pt) const {
        return by_color_[c] & by_type_[pt];
    }
    Bitboard occupied() const { return by_color_[BLACK] | by_color_[WHITE]; }
    Square king_square(Color c) const {
        assert(pieces(c, KING));
        return lsb(pieces(c, KING));
    }

    Bitboard attackers_to(Square s, Color by) const;
    bool in_check() const {
        return attackers_to(king_square(side_), ~side_) != 0;
    }

    // m must be pseudo-legal for this position (asserted in debug builds).
    void apply(Move m);

    // Internal invariants, including incremental key == recomputed key.
    bool is_consistent() const;

    std::string pretty() const;

   private:
    void put(Color c, PieceType pt, Square s);
    void remove(Color c, PieceType pt, Square s);
    uint64_t compute_key() const;

    std::array<Piece, SQUARE_NB> board_;
    Bitboard by_color_[COLOR_NB];
    Bitboard by_type_[PIECE_TYPE_NB];
    uint8_t hands_[COLOR_NB][HAND_TYPE_NB];
    Color side_;
    int ply_;
    uint64_t key_;
};

}  // namespace pioneer
