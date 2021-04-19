#include <iostream>
#include <vector>

#include "state.h"

const Move State::no_move = {std::make_pair(-1, -1), std::make_pair(-1, -1)};

//void State::set_State() {
//
//}
//
//void State::set_Result() {
//
//}
//
//Result State::get_Result() const {
//    return result_;
//}
//
//bool State::is_End() const {
//    return false;
//}
//
//
//int32_t State::get_turns() const {
//    return 0;
//}
//
//std::vector<Move> State::get_Moves() const {
//    return {};
//}
//
void State::do_Move(Move move) {
    return;
}

bool State::is_Draw() const {
    return false;
}
