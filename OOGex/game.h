#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <iostream>
#include <vector>

// mnk-game
namespace MNK {
const int M_SIZE = 15;
const int N_SIZE = 15;
const int K_SIZE = 5;

const int G_SIZE = M_SIZE * N_SIZE;
}  // namespace MNK

typedef int Move;
enum class Result : int { Unknown = 0, Black = 1, White = 2, Draw = 3 };

class State {
  std::vector<std::vector<char>> board;
  //std::vector<std::vector<char>> move_mask;
  //std::vector<std::vector<char>> threat_space;
  int player_to_move = int(Result::Black);
  int num_rows = MNK::M_SIZE;
  int num_cols = MNK::N_SIZE;
  int last_col = -1;
  int last_row = -1;
  Result result = Result::Unknown;

  bool check_connect();

 public:
  static const Move no_move;
  static const char player_markers[3];
  State();
  void do_move(Move move);
  bool has_moves() const;
  bool can_move(Move move) const;
  bool is_end() const;
  bool is_draw() const;
  int who_turns() const;
  bool isMask(Move move) const;
  bool isThreatX(Move move) const;
  bool isThreatO(Move move) const;
  std::vector<Move> get_moves() const;
  std::vector<Move> get_threat_moves() const;
  void print(std::ostream& os) const;
};

std::ostream& operator<<(std::ostream& os, const State& state);

#endif