#include <iostream>
#include <vector>

#include "state.h"

const Move State::no_move = {std::make_pair(-1, -1), std::make_pair(-1, -1)};

void State::set_State() {

}

void State::set_Result() {

}

Result State::get_Result() const {
    return result_;
}

bool State::is_End() const {
    return interface_->is_End();
}


int32_t State::get_turns() const {
    return interface_->get_turns();
}

std::vector<Move> State::get_Moves() const {
    return interface_->get_Move();
}

void State::do_Move(Move move) {
    interface_->MoveChess(move.from, move.to);
}

void State::set_Interface(Interface* param) {
    interface_ = param;
}

bool State::is_Draw() const {
    return false;
}
