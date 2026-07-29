#pragma once

#include <chrono>

#include "../agents/alphabeta_agent.h"

namespace pioneer {

// One definition shared by every test translation unit that needs
// private AlphaBetaAgent observability.
struct AlphaBetaAgentTestAccess {
    static int nonzero_killers(const AlphaBetaAgent& agent) {
        int count = 0;
        for (const auto& slots : agent.killers_)
            for (Move m : slots)
                if (m != MOVE_NONE) ++count;
        return count;
    }

    static int nonzero_history(const AlphaBetaAgent& agent) {
        int count = 0;
        for (const auto& by_origin : agent.history_)
            for (const auto& by_target : by_origin)
                for (uint16_t score : by_target)
                    if (score != 0) ++count;
        return count;
    }

    static void use_node_clock(AlphaBetaAgent& agent) {
        agent.now_ = [&agent] {
            return AlphaBetaAgent::SearchClock::time_point{} +
                   std::chrono::milliseconds(
                       static_cast<int64_t>(agent.nodes_));
        };
    }

    static void count_clock_calls(AlphaBetaAgent& agent, int& calls) {
        agent.now_ = [&calls] {
            ++calls;
            return AlphaBetaAgent::SearchClock::time_point{};
        };
    }

    static int completed_depth(const AlphaBetaAgent& agent) {
        return agent.completed_depth_;
    }

    static bool timed_out(const AlphaBetaAgent& agent) {
        return agent.timed_out_;
    }

    static uint64_t time_check_interval() {
        return AlphaBetaAgent::TIME_CHECK_INTERVAL;
    }

    static int exact_score(AlphaBetaAgent& agent, const Position& pos,
                           int depth, int ply = 0) {
        agent.nodes_ = 0;
        agent.timing_active_ = false;
        return agent.search(pos, depth, -MATE_SCORE - 1,
                            MATE_SCORE + 1, ply);
    }
};

}  // namespace pioneer
