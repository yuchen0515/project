#include "game.h"

#include <algorithm>

namespace pioneer {

bool Game::set(const std::string& sfen) {
    Position parsed;
    if (!parsed.set(sfen)) return false;

    pos_ = parsed;
    keys_.assign(1, pos_.key());
    moves_.clear();
    return true;
}

bool Game::apply(Move m) {
    if (m == MOVE_NONE || over()) return false;

    MoveList legal;
    generate_legal(pos_, legal);
    if (!legal.contains(m)) return false;

    pos_.apply(m);
    keys_.push_back(pos_.key());
    moves_.push_back(m);
    return true;
}

int Game::repetition_count() const {
    return int(std::count(keys_.begin(), keys_.end(), pos_.key()));
}

Outcome Game::outcome() const {
    // Repetition is checked first: it ends the game regardless of whose
    // turn it is, and always at the expense of the first player.
    if (repetition_count() >= REPETITION_LIMIT) return Outcome::WHITE_WIN;

    // No legal move loses, whether or not the king is attacked - shogi
    // scores stalemate as a loss rather than a draw.
    if (!has_legal_move(pos_))
        return pos_.side_to_move() == BLACK ? Outcome::WHITE_WIN
                                            : Outcome::BLACK_WIN;

    return Outcome::ONGOING;
}

const char* Game::termination() const {
    if (repetition_count() >= REPETITION_LIMIT) return "repetition";
    if (!has_legal_move(pos_)) return pos_.in_check() ? "checkmate" : "stalemate";
    return "";
}

}  // namespace pioneer
