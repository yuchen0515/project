#pragma once

#include <iostream>
#include <vector>

#include "../interface.h"

namespace MN {
    constexpr int32_t  M_SIZE = 7;
    constexpr int32_t  N_SIZE = 5;
    const int32_t G_SIZE = M_SIZE * N_SIZE;
}

enum class Result : int32_t {
    Upper = 0,
    Lower = 1,
    Draw = 2,
    Unknown = 3
};

#ifndef MOVE
#define MOVE

typedef struct move {
    std::pair<int32_t, int32_t> from;
    std::pair<int32_t, int32_t> to;
}Move;

#endif

//class Interface;

class State : public Interface {
    public:
        //State(){};
        //void set_State();
        //void set_Result();
        static const Move no_move;

        //Result get_Result() const;

        //bool is_End() const;
        //int32_t get_turns() const;

        //std::vector<Move> get_Moves() const;

        void do_Move(Move move);
        void run();

        Move Agent();
        bool is_Draw() const;

    //private:
        int32_t player_to_move_ = static_cast<int32_t> (Result::Lower);
        //Result result_ = Result::Unknown;
        int32_t turns_ = 1;
        bool agentDone_ = false;
        
        //bool isEnd_ = false;
        //std::vector<Move> move_;
};
