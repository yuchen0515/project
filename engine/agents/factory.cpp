#include "factory.h"

#include <cstdlib>

#include "alphabeta_agent.h"
#include "greedy_agent.h"
#include "random_agent.h"

namespace pioneer {

std::unique_ptr<Agent> make_agent(const std::string& name, uint64_t seed) {
    if (name == "random") return std::make_unique<RandomAgent>(seed);
    if (name == "greedy") return std::make_unique<GreedyAgent>(seed);

    // "alphabeta" defaults to depth 4; "alphabeta-d6" names the depth,
    // which matters because search depth is part of an agent's identity
    // for rating purposes.
    if (name.rfind("alphabeta", 0) == 0) {
        int depth = 4;
        if (const size_t d = name.find("-d"); d != std::string::npos)
            depth = std::atoi(name.c_str() + d + 2);
        const bool quiescence = name.find("-noqs") == std::string::npos;
        return std::make_unique<AlphaBetaAgent>(depth, seed, quiescence);
    }
    return nullptr;
}

std::vector<std::string> agent_names() {
    return {"random", "greedy", "alphabeta[-dN][-noqs]"};
}

}  // namespace pioneer
