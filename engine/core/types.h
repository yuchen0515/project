// Core types for the minishogi engine: colors, pieces, squares, moves.
//
// Board geometry. Squares are indexed 0..24, sq = rank * 5 + col.
// Rank 0 is the top of the board (USI rank 'a', White's back rank),
// rank 4 the bottom (USI rank 'e', Black's back rank). Col 0 is USI
// file 5, col 4 is file 1. Black (Sente) moves first and moves toward
// rank 0; White (Gote) toward rank 4.

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace pioneer {

enum Color : int8_t { BLACK = 0, WHITE = 1, COLOR_NB = 2 };

constexpr Color operator~(Color c) { return Color(c ^ 1); }

enum PieceType : int8_t {
    NO_PIECE_TYPE = 0,
    PAWN = 1, SILVER, BISHOP, ROOK, GOLD, KING,
    PRO_PAWN, PRO_SILVER, HORSE, DRAGON,
    PIECE_TYPE_NB = 11
};

constexpr bool is_promotable(PieceType pt) { return pt >= PAWN && pt <= ROOK; }
constexpr PieceType promoted(PieceType pt) { return PieceType(pt + 6); }
constexpr bool is_promoted(PieceType pt) { return pt >= PRO_PAWN; }
constexpr PieceType unpromoted(PieceType pt) {
    return is_promoted(pt) ? PieceType(pt - 6) : pt;
}

// Droppable piece types (what a hand can hold), indexed 0..4.
constexpr int HAND_TYPE_NB = 5;  // PAWN..GOLD
constexpr int hand_index(PieceType pt) {
    assert(pt >= PAWN && pt <= GOLD);
    return pt - PAWN;
}
constexpr PieceType hand_type(int idx) { return PieceType(PAWN + idx); }

// A piece is a color plus a piece type; 0 means empty.
enum Piece : int8_t { NO_PIECE = 0 };

constexpr Piece make_piece(Color c, PieceType pt) { return Piece((c << 4) | pt); }
constexpr Color color_of(Piece p) {
    assert(p != NO_PIECE);
    return Color(p >> 4);
}
constexpr PieceType type_of(Piece p) { return PieceType(p & 0xF); }

// Squares.
using Square = int8_t;
constexpr Square SQ_NONE = -1;
constexpr int SQUARE_NB = 25;
constexpr int RANK_NB = 5;
constexpr int FILE_NB = 5;

constexpr Square make_square(int rank, int col) { return Square(rank * 5 + col); }
constexpr int rank_of(Square s) { return s / 5; }
constexpr int col_of(Square s) { return s % 5; }
constexpr bool is_square(int rank, int col) {
    return rank >= 0 && rank < RANK_NB && col >= 0 && col < FILE_NB;
}

// The promotion zone is the single rank furthest from the player.
constexpr int promotion_rank(Color c) { return c == BLACK ? 0 : RANK_NB - 1; }
constexpr bool in_promotion_zone(Color c, Square s) {
    return rank_of(s) == promotion_rank(c);
}

// USI square names: file digit then rank letter, "5a" = square 0.
inline std::string usi_square(Square s) {
    std::string out;
    out += char('0' + (5 - col_of(s)));
    out += char('a' + rank_of(s));
    return out;
}
inline Square parse_usi_square(const std::string& str, size_t pos = 0) {
    if (pos + 1 >= str.size()) return SQ_NONE;
    int file = str[pos] - '0';
    int rank = str[pos + 1] - 'a';
    if (file < 1 || file > 5 || rank < 0 || rank >= RANK_NB) return SQ_NONE;
    return make_square(rank, 5 - file);
}

// Moves, packed in 16 bits:
//   bits 0-4   destination square
//   bits 5-9   origin square, or 25 + hand_index for drops
//   bit  10    promotion
// MOVE_NONE (0) never collides with a real move: origin 0 -> destination 0
// without promotion is never legal.
using Move = uint16_t;
constexpr Move MOVE_NONE = 0;

constexpr int DROP_ORIGIN = SQUARE_NB;

constexpr Move make_move(Square from, Square to, bool promo = false) {
    return Move(to | (from << 5) | (int(promo) << 10));
}
constexpr Move make_drop(PieceType pt, Square to) {
    return Move(to | ((DROP_ORIGIN + hand_index(pt)) << 5));
}
constexpr Square to_sq(Move m) { return Square(m & 0x1F); }
constexpr int origin_field(Move m) { return (m >> 5) & 0x1F; }
constexpr bool is_drop(Move m) { return origin_field(m) >= DROP_ORIGIN; }
constexpr Square from_sq(Move m) {
    assert(!is_drop(m));
    return Square(origin_field(m));
}
constexpr PieceType drop_type(Move m) {
    assert(is_drop(m));
    return hand_type(origin_field(m) - DROP_ORIGIN);
}
constexpr bool is_promotion(Move m) { return (m >> 10) & 1; }

// Piece letters as used by SFEN and USI drops (index = PieceType).
constexpr char PIECE_CHAR[PIECE_TYPE_NB] = {
    '?', 'P', 'S', 'B', 'R', 'G', 'K', '?', '?', '?', '?'};

inline PieceType piece_type_from_char(char c) {
    switch (c) {
        case 'P': return PAWN;
        case 'S': return SILVER;
        case 'B': return BISHOP;
        case 'R': return ROOK;
        case 'G': return GOLD;
        case 'K': return KING;
        default:  return NO_PIECE_TYPE;
    }
}

// USI move notation: "4e3d", "2e2a+" (promotion), "S*3c" (drop).
inline std::string usi_move(Move m) {
    if (m == MOVE_NONE) return "none";
    std::string out;
    if (is_drop(m)) {
        out += PIECE_CHAR[drop_type(m)];
        out += '*';
        out += usi_square(to_sq(m));
    } else {
        out += usi_square(from_sq(m));
        out += usi_square(to_sq(m));
        if (is_promotion(m)) out += '+';
    }
    return out;
}

inline Move parse_usi_move(const std::string& str) {
    if (str.size() < 4) return MOVE_NONE;
    if (str[1] == '*') {
        PieceType pt = piece_type_from_char(str[0]);
        Square to = parse_usi_square(str, 2);
        if (pt == NO_PIECE_TYPE || pt > GOLD || to == SQ_NONE) return MOVE_NONE;
        return make_drop(pt, to);
    }
    Square from = parse_usi_square(str, 0);
    Square to = parse_usi_square(str, 2);
    if (from == SQ_NONE || to == SQ_NONE) return MOVE_NONE;
    bool promo = str.size() > 4 && str[4] == '+';
    return make_move(from, to, promo);
}

}  // namespace pioneer
