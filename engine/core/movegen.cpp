#include "movegen.h"

namespace pioneer {

bool MoveList::contains(Move m) const {
    for (Move x : *this)
        if (x == m) return true;
    return false;
}

namespace {

void generate_board_moves(const Position& pos, MoveList& list) {
    const Color us = pos.side_to_move();
    const Bitboard occ = pos.occupied();
    const int promo_rank = promotion_rank(us);

    Bitboard from_bb = pos.pieces(us);
    while (from_bb) {
        const Square from = pop_lsb(from_bb);
        const PieceType pt = type_of(pos.piece_on(from));
        const bool from_in_zone = rank_of(from) == promo_rank;

        Bitboard targets = attacks_from(us, pt, from, occ) & ~pos.pieces(us);
        while (targets) {
            const Square to = pop_lsb(targets);
            const bool to_in_zone = rank_of(to) == promo_rank;

            // A pawn on the last rank would have no moves, so promotion
            // there is compulsory. Silver, bishop and rook always keep
            // moves, which makes promotion optional for them - and
            // declining it is a real tactical choice.
            const bool must_promote = (pt == PAWN && to_in_zone);
            if (!must_promote) list.add(make_move(from, to));
            if (is_promotable(pt) && (from_in_zone || to_in_zone))
                list.add(make_move(from, to, true));
        }
    }
}

void generate_drops(const Position& pos, MoveList& list) {
    const Color us = pos.side_to_move();
    const Bitboard empty = ~pos.occupied() & BOARD_BB;
    if (!empty) return;

    for (int i = 0; i < HAND_TYPE_NB; ++i) {
        const PieceType pt = hand_type(i);
        if (!pos.hand_count(us, pt)) continue;

        Bitboard targets = empty;
        if (pt == PAWN) {
            // A dropped pawn must keep a move, and two unpromoted pawns
            // may not share a file (nifu). Promoted pawns do not count.
            targets &= ~RANK_BB[promotion_rank(us)];
            Bitboard own_pawns = pos.pieces(us, PAWN);
            while (own_pawns) targets &= ~FILE_BB[col_of(pop_lsb(own_pawns))];
        }
        while (targets) list.add(make_drop(pt, pop_lsb(targets)));
    }
}

}  // namespace

void generate_pseudo_legal(const Position& pos, MoveList& list) {
    list.clear();
    generate_board_moves(pos, list);
    generate_drops(pos, list);
}

bool is_legal(const Position& pos, Move m) {
    const Color us = pos.side_to_move();
    Position next = pos;
    next.apply(m);

    if (next.attackers_to(next.king_square(us), ~us)) return false;

    // Uchifuzume: delivering mate by dropping a pawn loses on the spot,
    // so the move is simply illegal. Mate by any other drop, or by a
    // pawn *move*, is fine.
    if (is_drop(m) && drop_type(m) == PAWN && is_checkmate(next)) return false;

    return true;
}

void generate_legal(const Position& pos, MoveList& list) {
    MoveList pseudo;
    generate_pseudo_legal(pos, pseudo);
    list.clear();
    for (Move m : pseudo)
        if (is_legal(pos, m)) list.add(m);
}

bool has_legal_move(const Position& pos) {
    MoveList pseudo;
    generate_pseudo_legal(pos, pseudo);
    for (Move m : pseudo)
        if (is_legal(pos, m)) return true;
    return false;
}

bool gives_check(const Position& pos, Move m) {
    Position next = pos;
    next.apply(m);
    return next.in_check();
}

bool is_checkmate(const Position& pos) {
    return pos.in_check() && !has_legal_move(pos);
}

uint64_t perft(const Position& pos, int depth) {
    if (depth == 0) return 1;

    MoveList moves;
    generate_legal(pos, moves);
    if (depth == 1) return moves.size();

    uint64_t nodes = 0;
    for (Move m : moves) {
        Position next = pos;
        next.apply(m);
        nodes += perft(next, depth - 1);
    }
    return nodes;
}

}  // namespace pioneer
