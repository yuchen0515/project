#include "position.h"

#include <cctype>
#include <sstream>

namespace pioneer {

namespace {

// SFEN lists hand pieces strongest first.
constexpr PieceType HAND_ORDER[HAND_TYPE_NB] = {ROOK, BISHOP, GOLD, SILVER,
                                                PAWN};

// Only two of any piece type exist in minishogi, so a hand never holds
// more than two of a kind.
constexpr int MAX_HAND_COUNT = 2;

}  // namespace

void Position::put(Color c, PieceType pt, Square s) {
    assert(board_[s] == NO_PIECE);
    board_[s] = make_piece(c, pt);
    by_color_[c] |= square_bb(s);
    by_type_[pt] |= square_bb(s);
    key_ ^= zobrist::KEYS.psq[c][pt][s];
}

void Position::remove(Color c, PieceType pt, Square s) {
    assert(board_[s] == make_piece(c, pt));
    board_[s] = NO_PIECE;
    by_color_[c] &= ~square_bb(s);
    by_type_[pt] &= ~square_bb(s);
    key_ ^= zobrist::KEYS.psq[c][pt][s];
}

bool Position::hand_empty(Color c) const {
    for (int t = 0; t < HAND_TYPE_NB; ++t)
        if (hands_[c][t]) return false;
    return true;
}

bool Position::set(const std::string& sfen) {
    board_.fill(NO_PIECE);
    for (int c = 0; c < COLOR_NB; ++c) {
        by_color_[c] = 0;
        for (int t = 0; t < HAND_TYPE_NB; ++t) hands_[c][t] = 0;
    }
    for (int pt = 0; pt < PIECE_TYPE_NB; ++pt) by_type_[pt] = 0;
    side_ = BLACK;
    ply_ = 1;
    key_ = 0;

    std::istringstream is(sfen);
    std::string board_field, side_field, hand_field;
    if (!(is >> board_field >> side_field >> hand_field)) return false;

    int rank = 0, col = 0;
    bool pending_promotion = false;
    for (char ch : board_field) {
        if (ch == '/') {
            if (col != FILE_NB || pending_promotion || rank + 1 >= RANK_NB)
                return false;
            ++rank;
            col = 0;
        } else if (ch == '+') {
            if (pending_promotion) return false;
            pending_promotion = true;
        } else if (ch >= '1' && ch <= '5') {
            if (pending_promotion) return false;
            col += ch - '0';
            if (col > FILE_NB) return false;
        } else {
            PieceType pt = piece_type_from_char(char(std::toupper(ch)));
            if (pt == NO_PIECE_TYPE || col >= FILE_NB) return false;
            if (pending_promotion) {
                if (!is_promotable(pt)) return false;
                pt = promoted(pt);
                pending_promotion = false;
            }
            Color c = std::isupper(static_cast<unsigned char>(ch)) ? BLACK : WHITE;
            if (pt == KING && pieces(c, KING)) return false;
            put(c, pt, make_square(rank, col));
            ++col;
        }
    }
    if (rank != RANK_NB - 1 || col != FILE_NB || pending_promotion) return false;

    if (side_field == "w") {
        side_ = WHITE;
        key_ ^= zobrist::KEYS.side;
    } else if (side_field != "b") {
        return false;
    }

    if (hand_field != "-") {
        int count = 0;
        for (char ch : hand_field) {
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                count = count * 10 + (ch - '0');
                if (count > MAX_HAND_COUNT) return false;
                continue;
            }
            PieceType pt = piece_type_from_char(char(std::toupper(ch)));
            if (pt == NO_PIECE_TYPE || pt > GOLD) return false;
            Color c = std::isupper(static_cast<unsigned char>(ch)) ? BLACK : WHITE;
            uint8_t& slot = hands_[c][hand_index(pt)];
            if (slot) return false;  // same piece listed twice
            slot = uint8_t(count ? count : 1);
            count = 0;
        }
        if (count) return false;
    }
    for (int c = 0; c < COLOR_NB; ++c)
        for (int t = 0; t < HAND_TYPE_NB; ++t)
            for (int n = 1; n <= hands_[c][t]; ++n)
                key_ ^= zobrist::KEYS.hand[c][t][n];

    std::string ply_field;
    if (is >> ply_field) {
        try {
            ply_ = std::stoi(ply_field);
        } catch (const std::exception&) {
            return false;
        }
        if (ply_ < 1) return false;
    }

    return pieces(BLACK, KING) && pieces(WHITE, KING);
}

std::string Position::sfen() const {
    std::string out;
    for (int rank = 0; rank < RANK_NB; ++rank) {
        int empty = 0;
        for (int col = 0; col < FILE_NB; ++col) {
            Piece p = board_[make_square(rank, col)];
            if (p == NO_PIECE) {
                ++empty;
                continue;
            }
            if (empty) {
                out += char('0' + empty);
                empty = 0;
            }
            PieceType pt = type_of(p);
            if (is_promoted(pt)) out += '+';
            char letter = PIECE_CHAR[unpromoted(pt)];
            out += color_of(p) == BLACK
                       ? letter
                       : char(std::tolower(static_cast<unsigned char>(letter)));
        }
        if (empty) out += char('0' + empty);
        if (rank != RANK_NB - 1) out += '/';
    }

    out += side_ == BLACK ? " b " : " w ";

    std::string hand;
    for (int c = 0; c < COLOR_NB; ++c)
        for (PieceType pt : HAND_ORDER) {
            int n = hands_[c][hand_index(pt)];
            if (!n) continue;
            if (n > 1) hand += char('0' + n);
            hand += c == BLACK ? PIECE_CHAR[pt]
                               : char(std::tolower(
                                     static_cast<unsigned char>(PIECE_CHAR[pt])));
        }
    out += hand.empty() ? "-" : hand;

    out += ' ';
    out += std::to_string(ply_);
    return out;
}

Bitboard Position::attackers_to(Square s, Color by) const {
    const Bitboard occ = occupied();
    // For every step pattern here, "X on x attacks s" is equivalent to
    // "a piece of the opposite color on s attacks x", so the tables can
    // be read backwards from s.
    const Color rev = ~by;

    Bitboard att = bishop_attacks(s, occ) &
                   (pieces(by, BISHOP) | pieces(by, HORSE));
    att |= rook_attacks(s, occ) & (pieces(by, ROOK) | pieces(by, DRAGON));
    att |= STEP_ATTACKS[rev][PAWN][s] & pieces(by, PAWN);
    att |= STEP_ATTACKS[rev][SILVER][s] & pieces(by, SILVER);
    att |= STEP_ATTACKS[rev][GOLD][s] &
           (pieces(by, GOLD) | pieces(by, PRO_PAWN) | pieces(by, PRO_SILVER));
    att |= STEP_ATTACKS[rev][KING][s] & pieces(by, KING);
    att |= STEP_ATTACKS[rev][HORSE][s] & pieces(by, HORSE);
    att |= STEP_ATTACKS[rev][DRAGON][s] & pieces(by, DRAGON);
    return att;
}

void Position::apply(Move m) {
    assert(m != MOVE_NONE);
    const Color us = side_;
    const Square to = to_sq(m);

    if (is_drop(m)) {
        const PieceType pt = drop_type(m);
        uint8_t& slot = hands_[us][hand_index(pt)];
        assert(slot > 0 && board_[to] == NO_PIECE);
        key_ ^= zobrist::KEYS.hand[us][hand_index(pt)][slot];
        --slot;
        put(us, pt, to);
    } else {
        const Square from = from_sq(m);
        const Piece moving = board_[from];
        assert(moving != NO_PIECE && color_of(moving) == us);
        PieceType pt = type_of(moving);

        if (Piece captured = board_[to]; captured != NO_PIECE) {
            assert(color_of(captured) == ~us);
            const PieceType base = unpromoted(type_of(captured));
            assert(base != KING);
            remove(~us, type_of(captured), to);
            uint8_t& slot = hands_[us][hand_index(base)];
            ++slot;
            assert(slot <= MAX_HAND_COUNT);
            key_ ^= zobrist::KEYS.hand[us][hand_index(base)][slot];
        }

        remove(us, pt, from);
        if (is_promotion(m)) {
            assert(is_promotable(pt));
            pt = promoted(pt);
        }
        put(us, pt, to);
    }

    side_ = ~us;
    key_ ^= zobrist::KEYS.side;
    ++ply_;
}

uint64_t Position::compute_key() const {
    uint64_t k = 0;
    for (int s = 0; s < SQUARE_NB; ++s)
        if (Piece p = board_[s]; p != NO_PIECE)
            k ^= zobrist::KEYS.psq[color_of(p)][type_of(p)][s];
    for (int c = 0; c < COLOR_NB; ++c)
        for (int t = 0; t < HAND_TYPE_NB; ++t)
            for (int n = 1; n <= hands_[c][t]; ++n)
                k ^= zobrist::KEYS.hand[c][t][n];
    if (side_ == WHITE) k ^= zobrist::KEYS.side;
    return k;
}

bool Position::is_consistent() const {
    Bitboard seen = 0;
    for (int s = 0; s < SQUARE_NB; ++s) {
        Piece p = board_[s];
        if (p == NO_PIECE) {
            if (occupied() & square_bb(Square(s))) return false;
            continue;
        }
        seen |= square_bb(Square(s));
        if (!(by_color_[color_of(p)] & square_bb(Square(s)))) return false;
        if (!(by_type_[type_of(p)] & square_bb(Square(s)))) return false;
    }
    if (seen != occupied()) return false;
    if (by_color_[BLACK] & by_color_[WHITE]) return false;

    Bitboard union_types = 0;
    for (int pt = PAWN; pt < PIECE_TYPE_NB; ++pt) {
        if (by_type_[pt] & ~occupied()) return false;
        if (union_types & by_type_[pt]) return false;
        union_types |= by_type_[pt];
    }
    if (union_types != occupied()) return false;

    if (popcount(pieces(BLACK, KING)) > 1 || popcount(pieces(WHITE, KING)) > 1)
        return false;
    for (int c = 0; c < COLOR_NB; ++c)
        for (int t = 0; t < HAND_TYPE_NB; ++t)
            if (hands_[c][t] > MAX_HAND_COUNT) return false;

    return key_ == compute_key();
}

std::string Position::pretty() const {
    std::string out = "  5 4 3 2 1\n";
    for (int rank = 0; rank < RANK_NB; ++rank) {
        out += char('a' + rank);
        for (int col = 0; col < FILE_NB; ++col) {
            Piece p = board_[make_square(rank, col)];
            if (p == NO_PIECE) {
                out += " .";
                continue;
            }
            char letter = PIECE_CHAR[unpromoted(type_of(p))];
            if (color_of(p) == WHITE)
                letter = char(std::tolower(static_cast<unsigned char>(letter)));
            out += is_promoted(type_of(p)) ? '+' : ' ';
            out += letter;
        }
        out += '\n';
    }
    for (int c = 0; c < COLOR_NB; ++c) {
        out += c == BLACK ? "black hand:" : "white hand:";
        if (hand_empty(Color(c))) out += " -";
        for (PieceType pt : HAND_ORDER)
            for (int n = 0; n < hands_[c][hand_index(pt)]; ++n) {
                out += ' ';
                out += PIECE_CHAR[pt];
            }
        out += '\n';
    }
    out += side_ == BLACK ? "black to move" : "white to move";
    out += ", ply " + std::to_string(ply_) + "\n";
    return out;
}

}  // namespace pioneer
