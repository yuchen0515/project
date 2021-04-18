#include <iostream>
#include <vector>

#include "state.h"

void State::set_State() {

}

void State::set_Result() {

}

Result State::get_Result() const {
    return result_;
}

void State::set_End() {

}

bool State::is_End() const {
    return isEnd_;
}

void State::set_turns() {

}

int32_t State::get_turns() const {
    return turns_;
}

void State::set_Moves() {
    move_.clear();

}

const std::vector<Move>& State::get_Moves() const {
    return move_;
}

void State::do_Move(Move move) {
    Interface->do_Move(move);
}

void State::set_Interface(Interface* param) {

}

bool State::is_Draw() const {
    return false;
}
