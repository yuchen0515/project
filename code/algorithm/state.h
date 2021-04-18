#pragma once

#include <iostream>
#include <vector>

#include "../interface.h"

class Interface;

namespace RC {
    constexpr int32_t  ROW_SIZE = 9;
    constexpr int32_t  COL_SIZE = 5;
    const int32_t G_SIZE = ROW_SIZE * COL_SIZE;
}

enum class Result : int32_t {
    Upper = 0,
    Lower = 1,
    Unknown = 2
};

typedef struct move{
    std::pair<int32_t, int32_t> from;
    std::pair<int32_t, int32_t> to;
}Move;

class State {
    public:
        void set_State() {

        }


        void set_Result() {

        }

        Result get_Result() const {
            return result_;
        }

        void set_End() {

        }

        bool is_End() const {
            return isEnd_;
        }

        void set_turns() {

        }

        int32_t get_turns() const {
            return turns_;
        }

        void set_Moves() {
            move_.clear();

        }
        
        const std::vector<Move>& get_Moves() const {
            return move_;
        }

        void do_Move(Move move) {
            //Interface->do_Move(move);
        }

        void set_Interface(Interface* param) {

        }

        bool is_Draw() const {
            return false;
        }

    private:
        class Interface *interface_;
        int32_t player_to_move_ = static_cast<int32_t> (Result::Lower);
        Result result_ = Result::Unknown;
        int32_t turns_ = 0;
        bool isEnd_ = false;
        std::vector<Move> move_;
};
