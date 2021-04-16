#include "stdafx.h"
#include "game.h"

#include <cassert>
#include <iostream>
using namespace std;


const Move State::no_move = -1;
const char State::player_markers[3] = {'.', 'X', 'O'};
const int threatDis = 2;
const int threatCon = 2;


State::State() {
    board.resize(num_rows, vector<char>(num_cols, player_markers[0]));
}

//檢查連五，就是遊戲結束的條件
bool State::check_connect() {
    auto piece = board[last_row][last_col];

    // X X X
    int left = 0, right = 0;
    for (int col = last_col - 1; col >= last_col - 4 && col >= 0 && board[last_row][col] == piece; --col)
        left++;
    for (int col = last_col + 1; col <= last_col + 4 && col < num_cols && board[last_row][col] == piece; ++col)
        right++;
    if (left + 1 + right >= MNK::K_SIZE) {
        return true;
    }
    // X
    // X
    // X
    int up = 0, down = 0;
    for (int row = last_row - 1; row >= last_row - 4 && row >= 0 && board[row][last_col] == piece; --row)
        up++;
    for (int row = last_row + 1; row <= last_row + 4 && row < num_rows && board[row][last_col] == piece; ++row)
        down++;
    if (up + 1 + down >= MNK::K_SIZE) {
        return true;
    }

    // X
    //  X
    //   X
    up = 0;
    down = 0;
    for (int row = last_row - 1, col = last_col - 1; 
            row >= last_row - 4 && col >= last_col - 4 && row >= 0 && col >= 0 && board[row][col] == piece;
            --row, --col)
        up++;
    for (int row = last_row + 1, col = last_col + 1;
            row <= last_row + 4 && col <= last_col + 4 && row < num_rows && col < num_cols && board[row][col] == piece;
            ++row, ++col)
        down++;
    if (up + 1 + down >= MNK::K_SIZE) {
        return true;
    }

    //   X
    //  X
    // X
    up = 0;
    down = 0;
    for (int row = last_row - 1, col = last_col + 1;
            row >= last_row - 4 && col <= last_col + 4 && row >= 0 && col < num_cols && board[row][col] == piece;
            --row, ++col)
        up++;
    for (int row = last_row + 1, col = last_col - 1;
            row <= last_row + 4 && col >= last_col - 4 && row < num_rows && col >= 0 && board[row][col] == piece;
            ++row, --col)
        down++;
    if (up + 1 + down >= MNK::K_SIZE) {
        return true;
    }

    return false;
}

void State::do_move(Move move) {
    assert(move >= 0 && move < MNK::G_SIZE);
    last_col = move % num_cols;
    last_row = move / num_cols;
    assert(board[last_row][last_col] == player_markers[0]);

    board[last_row][last_col] = player_markers[player_to_move];

    if (check_connect()) {
        if (player_to_move == int(Result::Black)) {
            result = Result::Black;
        } else {
            result = Result::White;
        }
    } else if (!has_moves()) {
        result = Result::Draw;
    }
    // 1 -> 2, 2 -> 1
    player_to_move = 3 - player_to_move;
}

bool State::can_move(Move move) const {
    return board[move / num_cols][move % num_cols] == player_markers[0];
}

bool State::has_moves() const {
    if (result != Result::Unknown) return false;
    for (int grid = 0; grid < MNK::G_SIZE; grid++) {
        if (board[grid / num_cols][grid % num_cols] == player_markers[0]) {
            return true;
        }
    }
    return false;
}

bool State::is_end() const { return result != Result::Unknown; }

bool State::is_draw() const { return result == Result::Draw; }

int State::who_turns() const { return player_to_move; }

//定義迫著，這裡X與0分開寫
bool State::isThreatX(Move move) const {
    int i = -1, j = -1;
    i = move / num_rows;
    j = move % num_cols;

    // X X X
    // check left side for X
    int left = 0;
    for (int col = j - 1; col >= j - threatDis && col >= 0 && board[i][col] == player_markers[1]; --col){
        left++;
    }
    // check right side for X
    int right = 0;
    for (int col = j + 1; col <= j + threatDis && col < num_cols && board[i][col] == player_markers[1]; ++col){

        right++;

    }

    if (left + right >= threatCon){
        return true;
    }

    // X
    // X
    // X
    // check up side for X
    int up = 0;
    for (int row = i - 1; row >= i - threatDis && row >= 0 && board[row][j] == player_markers[1]; --row){

        up++;

    }
    // check down side for X
    int down = 0;
    for (int row = i + 1; row <= i + threatDis && row < num_rows && board[row][j] == player_markers[1]; ++row){

        down++;

    }

    if (up + down >= threatCon){
        return true;
    }

    // X 
    //  X
    //   X
    // check upper left for X
    int upperLeft = 0;
    for (int row = i - 1, col = j - 1;
            row >= i - threatDis && row >= 0 && col >= j - threatDis && col >= 0 && board[row][col] == player_markers[1];
            --row, --col){

        upperLeft++;

    }
    // check lower right for X
    int lowerRight = 0;
    for (int row = i + 1, col = j + 1;
            row <= i + threatDis && row < num_rows && col <= j + threatDis && col < num_cols && board[row][col] == player_markers[1];
            ++row, ++col){

        lowerRight++;

    }

    if (upperLeft + lowerRight >= threatCon){
        return true;
    }

    //   X
    //  X
    // X 	
    // check lower left for X
    int lowerLeft = 0;
    for (int row = i + 1, col = j - 1;
            row <= i + threatDis && row < num_rows && col >= j - threatDis && col >= 0 && board[row][col] == player_markers[1];
            ++row, --col){

        lowerLeft++;

    }
    // check upper right for X
    int upperRight = 0;
    for (int row = i - 1, col = j + 1;
            row >= i - threatDis && row >= 0 && col <= j + threatDis && col < num_cols && board[row][col] == player_markers[1];
            --row, ++col){

        upperRight++;

    }

    if (lowerLeft + upperRight >= threatCon){
        return true;
    }

    return false;
}

bool State::isThreatO(Move move) const {
    int i = -1, j = -1;
    i = move / num_rows;
    j = move % num_cols;

    // O O O
    // check left side for O
    int left = 0;
    for (int col = j - 1; col >= j - threatDis && col >= 0 && board[i][col] == player_markers[2]; --col){

        left++;

    }
    // check right side for O
    int right = 0;
    for (int col = j + 1; col <= j + threatDis && col < num_cols && board[i][col] == player_markers[2]; ++col){

        right++;

    }

    if (left + right >= threatCon){
        return true;
    }

    // O
    // O
    // O
    // check up side for O
    int up = 0;
    for (int row = i - 1; row >= i - threatDis && row >= 0 && board[row][j] == player_markers[2]; --row){

        up++;

    }
    // check down side for O
    int down = 0;
    for (int row = i + 1; row <= i + threatDis && row < num_rows && board[row][j] == player_markers[2]; ++row){

        down++;

    }

    if (up + down >= threatCon){
        return true;
    }

    // O 
    //  O
    //   O
    // check upper left for O
    int upperLeft = 0;
    for (int row = i - 1, col = j - 1;
            row >= i - threatDis && row >= 0 && col >= j - threatDis && col >= 0 && board[row][col] == player_markers[2];
            --row, --col){

        upperLeft++;

    }
    // check lower right for O
    int lowerRight = 0;
    for (int row = i + 1, col = j + 1;
            row <= i + threatDis && row < num_rows && col <= j + threatDis && col < num_cols && board[row][col] == player_markers[2];
            ++row, ++col){

        lowerRight++;

    }

    if (upperLeft + lowerRight >= threatCon){
        return true;
    }

    //   O
    //  O
    // O 	
    // check lower left for O
    int lowerLeft = 0;
    for (int row = i + 1, col = j - 1;
            row <= i + threatDis && row < num_rows && col >= j - threatDis && col >= 0 && board[row][col] == player_markers[2];
            ++row, --col){

        lowerLeft++;

    }
    // check upper right for O
    int upperRight = 0;
    for (int row = i - 1, col = j + 1;
            row >= i - threatDis && row >= 0 && col <= j + threatDis && col < num_cols && board[row][col] == player_markers[2];
            --row, ++col){

        upperRight++;

    }

    if (lowerLeft + upperRight >= threatCon){
        return true;
    }

    return false;
}

//取得一般合法走步
vector<Move> State::get_moves() const {
    vector<Move> moves;
    if (is_end()) return moves;

    for (int grid = 0; grid < MNK::G_SIZE; grid++) {
        if (can_move(grid)) {
            moves.push_back(grid);
        }
    }

    return moves;
}

////取得迫著空間走步
vector<Move> State::get_threat_moves() const {
    vector<Move> moves;
    if (is_end()) return moves;

    //check threat
    for (int grid = 0; grid < MNK::G_SIZE; grid++){
        if (can_move(grid)) {
            if (isThreatX(grid)){
                moves.push_back(grid);
                continue;
            }
            if (isThreatO(grid)){
                moves.push_back(grid);
            }
        }
    }

    //no threat space
    if (moves.size() == 0){
        for (int grid = 0; grid < MNK::G_SIZE; grid++) {
            if (can_move(grid)) {
                moves.push_back(grid);
            }
        }
    }

    return moves;
}

//印出棋盤狀態
void State::print(ostream& os) const {
    os << endl;
    os << "  ";
    for (int col = 0; col < num_cols ; ++col) {
        os << (char)(col + 65) << ' ';
    }
    os << endl;
    os << "+-";
    for (int col = 0; col < num_cols; ++col) {
        os << "--";
    }
    os << "+" << endl;
    for (int row = 0; row < num_rows; ++row) {
        os << "| ";
        for (int col = 0; col < num_cols; ++col) {
            os << board[row][col] << ' ';
        }
        os << "| " << num_rows - row << endl;
    }
    os << "+-";
    for (int col = 0; col < num_cols; ++col) {
        os << "--";
    }
    os << "+" << endl;
    os << player_markers[player_to_move] << " to move " << endl;
    if (is_end()) {
        if (result == Result::Black) {
            os << "X win" << endl;
        } else if (result == Result::White) {
            os << "O win" << endl;
        } else {
            os << "Draw" << endl;
        }
    }
}

ostream& operator<<(ostream& os, const State& state) {
    state.print(os);
    return os;
}
