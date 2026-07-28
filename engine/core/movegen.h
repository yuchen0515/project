// Move generation and the rules that decide what is actually legal.

#pragma once

#include "position.h"

namespace pioneer {

// Generous upper bound: drops top out at 5 hand types x 24 empty
// squares, board moves at well under 80 including promotions.
constexpr int MAX_MOVES = 256;

class MoveList {
   public:
    void add(Move m) {
        assert(size_ < MAX_MOVES);
        moves_[size_++] = m;
    }
    void clear() { size_ = 0; }
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    Move operator[](int i) const { return moves_[i]; }
    const Move* begin() const { return moves_; }
    const Move* end() const { return moves_ + size_; }
    bool contains(Move m) const;

   private:
    Move moves_[MAX_MOVES];
    int size_ = 0;
};

// Board moves and drops that respect piece movement, the two static
// drop bans (nifu, pawn on the last rank) and forced pawn promotion,
// but not king safety or uchifuzume.
void generate_pseudo_legal(const Position& pos, MoveList& list);

// Pseudo-legal moves minus those leaving the mover in check, minus
// pawn drops that deliver mate.
void generate_legal(const Position& pos, MoveList& list);

bool is_legal(const Position& pos, Move m);
bool has_legal_move(const Position& pos);
bool gives_check(const Position& pos, Move m);
bool is_checkmate(const Position& pos);

uint64_t perft(const Position& pos, int depth);

}  // namespace pioneer
