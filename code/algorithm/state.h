#pragma once

#include <iostream>
#include <vector>

#include "../interface.h"

namespace MN {
    constexpr int32_t  M_SIZE = 7;
    constexpr int32_t  N_SIZE = 5;
    const int32_t G_SIZE = M_SIZE * N_SIZE;
}

// enum ==> 結果
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
    double value;
}Move;

#endif

//class Interface;

class State : public Interface {
    public:
        //State(){};
        //void set_State();
        //void set_Result();
        //Result get_Result() const;
        //bool is_End() const;
        //int32_t get_turns() const;
        //std::vector<Move> get_Moves() const;
        
        static const Move no_move;

        void do_Move(Move move);
        void run();

        Move Agent(
                const int32_t lowerSimul,
                const int32_t upperSimul);

        bool is_Draw() const;

        enum class PLAYER_TYPE_: int32_t {
            AGENT_,
            PLAYER_};
        //private:
        //int32_t player_to_move_ = static_cast<int32_t> (Result::Lower);
        //int32_t turns_ = LOWER_;
        bool agentDone_ = false;

        // 設定對局型態:
        // Player or Agnet
        PLAYER_TYPE_ player_setting_[2] = {
            PLAYER_TYPE_::PLAYER_, 
            PLAYER_TYPE_::PLAYER_};
        //Result result_ = Result::Unknown;
        //bool isEnd_ = false;
        //std::vector<Move> move_;

        std::vector<int32_t> winner_ = {0, 0};
        std::vector<int32_t> lose_ = {0, 0};
        int32_t game_number_ = 1e5;

        double sigmoid(const double score) const;
        double step_tangent(const double score) const;
        double get_board_score(const int32_t player_type) const;
        void switch_turn();
};
