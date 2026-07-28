// Game: a position plus the history needed for the rules that depend
// on it, and the verdict on whether play is over.

#pragma once

#include <string>
#include <vector>

#include "movegen.h"

namespace pioneer {

enum class Outcome { ONGOING, BLACK_WIN, WHITE_WIN };

// Minishogi ends on the fourth occurrence of a position (same board,
// same hands, same side to move) with the player who moved first
// losing - unlike shogi, where sennichite is a draw to be replayed.
constexpr int REPETITION_LIMIT = 4;

class Game {
   public:
    explicit Game(const std::string& sfen = START_SFEN) { set(sfen); }

    bool set(const std::string& sfen);

    const Position& position() const { return pos_; }
    // The position the game started from, so a driver can hand an
    // external engine the full history rather than a bare snapshot.
    const std::string& initial_sfen() const { return initial_sfen_; }
    Color side_to_move() const { return pos_.side_to_move(); }

    // Rejects anything not legal in the current position.
    bool apply(Move m);
    bool apply_usi(const std::string& usi) { return apply(parse_usi_move(usi)); }

    // How many times the current position has occurred, itself included.
    int repetition_count() const;

    Outcome outcome() const;
    bool over() const { return outcome() != Outcome::ONGOING; }

    // Why the game ended, for logs and match records: "checkmate",
    // "stalemate", "repetition", or "" while play continues.
    const char* termination() const;

    const std::vector<Move>& moves() const { return moves_; }
    int ply() const { return int(moves_.size()); }

   private:
    Position pos_;
    std::string initial_sfen_;
    std::vector<uint64_t> keys_;  // one per position, starting position first
    std::vector<Move> moves_;
};

}  // namespace pioneer
