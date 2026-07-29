#include "movegen.h"

namespace pioneer {

bool MoveList::contains(Move m) const {
    for (Move x : *this)
        if (x == m) return true;
    return false;
}

namespace {

enum class GenMode { COLLECT_ALL, STOP_AFTER_FIRST };

struct LegalState {
    Color us;
    Color them;
    Square king;
    Bitboard occupied;
    Bitboard checkers;
    Bitboard pinned;
    Bitboard board_evasion_targets;
    Bitboard drop_evasion_targets;
    Bitboard checking_pawn_drop_targets;
    std::array<Bitboard, SQUARE_NB> pin_targets{};
};

bool emit_move(MoveList& list, Move m, GenMode mode) {
    list.add(m);
    return mode == GenMode::STOP_AFTER_FIRST;
}

void generate_pseudo_board_moves(const Position& pos, MoveList& list) {
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

void generate_pseudo_drops(const Position& pos, MoveList& list) {
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

LegalState analyze_king(const Position& pos) {
    LegalState state{};
    state.us = pos.side_to_move();
    state.them = ~state.us;
    state.king = pos.king_square(state.us);
    state.occupied = pos.occupied();
    state.checkers = pos.attackers_to(state.king, state.them);
    state.checking_pawn_drop_targets =
        STEP_ATTACKS[state.them][PAWN][pos.king_square(state.them)];

    const int check_count = popcount(state.checkers);
    if (check_count == 0) {
        state.board_evasion_targets = BOARD_BB;
        state.drop_evasion_targets = BOARD_BB;
    } else if (check_count == 1) {
        const Square checker = lsb(state.checkers);
        const Bitboard between = BETWEEN_BB[state.king][checker];
        state.board_evasion_targets = between | square_bb(checker);
        state.drop_evasion_targets = between;
    }

    // In double check only the king can move, so pin information cannot
    // affect the generated list.
    if (check_count >= 2) return state;

    Bitboard snipers =
        rook_attacks(state.king, 0) &
        (pos.pieces(state.them, ROOK) | pos.pieces(state.them, DRAGON));
    snipers |=
        bishop_attacks(state.king, 0) &
        (pos.pieces(state.them, BISHOP) | pos.pieces(state.them, HORSE));

    const Bitboard ours = pos.pieces(state.us);
    while (snipers) {
        const Square pinner = pop_lsb(snipers);
        const Bitboard between = BETWEEN_BB[state.king][pinner];
        const Bitboard blockers = between & state.occupied;
        if (popcount(blockers) != 1 || !(blockers & ours)) continue;

        const Square pinned = lsb(blockers);
        state.pinned |= blockers;
        state.pin_targets[pinned] = between | square_bb(pinner);
    }

    return state;
}

bool king_destination_is_safe(const Position& pos, const LegalState& state,
                              Square from, Square to) {
    const Bitboard occupied =
        (state.occupied & ~square_bb(from)) | square_bb(to);
    return !pos.attackers_to(to, state.them, occupied);
}

bool illegal_pawn_drop_mate(const Position& pos, Square to) {
    Position next = pos;
    next.apply(make_drop(PAWN, to));
    // The caller has already established that the dropped pawn gives
    // check, so mate is exactly the absence of a legal reply.
    return !has_legal_move(next);
}

bool generate_legal_board_moves(const Position& pos, const LegalState& state,
                                MoveList& list, GenMode mode) {
    const int promo_rank = promotion_rank(state.us);

    Bitboard from_bb = pos.pieces(state.us);
    while (from_bb) {
        const Square from = pop_lsb(from_bb);
        const PieceType pt = type_of(pos.piece_on(from));
        Bitboard targets =
            attacks_from(state.us, pt, from, state.occupied) &
            ~pos.pieces(state.us);

        if (pt == KING) {
            while (targets) {
                const Square to = pop_lsb(targets);
                if (!king_destination_is_safe(pos, state, from, to)) continue;
                if (emit_move(list, make_move(from, to), mode)) return true;
            }
            continue;
        }

        targets &= state.board_evasion_targets;
        if (state.pinned & square_bb(from))
            targets &= state.pin_targets[from];

        const bool from_in_zone = rank_of(from) == promo_rank;
        while (targets) {
            const Square to = pop_lsb(targets);
            const bool to_in_zone = rank_of(to) == promo_rank;
            const bool must_promote = pt == PAWN && to_in_zone;

            if (!must_promote &&
                emit_move(list, make_move(from, to), mode))
                return true;
            if (is_promotable(pt) && (from_in_zone || to_in_zone) &&
                emit_move(list, make_move(from, to, true), mode))
                return true;
        }
    }
    return false;
}

bool generate_legal_drops(const Position& pos, const LegalState& state,
                          MoveList& list, GenMode mode) {
    const Bitboard empty =
        (~state.occupied & BOARD_BB) & state.drop_evasion_targets;
    if (!empty) return false;

    for (int i = 0; i < HAND_TYPE_NB; ++i) {
        const PieceType pt = hand_type(i);
        if (!pos.hand_count(state.us, pt)) continue;

        Bitboard targets = empty;
        if (pt == PAWN) {
            targets &= ~RANK_BB[promotion_rank(state.us)];
            Bitboard own_pawns = pos.pieces(state.us, PAWN);
            while (own_pawns)
                targets &= ~FILE_BB[col_of(pop_lsb(own_pawns))];
        }

        while (targets) {
            const Square to = pop_lsb(targets);
            if (pt == PAWN &&
                (state.checking_pawn_drop_targets & square_bb(to)) &&
                illegal_pawn_drop_mate(pos, to))
                continue;
            if (emit_move(list, make_drop(pt, to), mode)) return true;
        }
    }
    return false;
}

bool generate_legal_impl(const Position& pos, MoveList& list, GenMode mode) {
    list.clear();
    const LegalState state = analyze_king(pos);
    if (generate_legal_board_moves(pos, state, list, mode)) return true;
    if (generate_legal_drops(pos, state, list, mode)) return true;
    return !list.empty();
}

}  // namespace

void generate_pseudo_legal(const Position& pos, MoveList& list) {
    list.clear();
    generate_pseudo_board_moves(pos, list);
    generate_pseudo_drops(pos, list);
}

bool is_legal(const Position& pos, Move m) {
    const Color us = pos.side_to_move();
    const bool checking_pawn_drop =
        is_drop(m) && drop_type(m) == PAWN &&
        (STEP_ATTACKS[us][PAWN][to_sq(m)] &
         square_bb(pos.king_square(~us)));

    Position next = pos;
    next.apply(m);

    if (next.attackers_to(next.king_square(us), ~us)) return false;

    // Uchifuzume: delivering mate by dropping a pawn is illegal. Mate by
    // any other drop, or by a pawn *move*, is fine.
    if (checking_pawn_drop && !has_legal_move(next)) return false;

    return true;
}

void generate_legal(const Position& pos, MoveList& list) {
    generate_legal_impl(pos, list, GenMode::COLLECT_ALL);
}

bool has_legal_move(const Position& pos) {
    MoveList first;
    return generate_legal_impl(pos, first, GenMode::STOP_AFTER_FIRST);
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
