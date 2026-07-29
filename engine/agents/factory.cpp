#include "factory.h"

#include <cstdlib>

#include "../arena/external_engine.h"
#include "alphabeta_agent.h"
#include "greedy_agent.h"
#include "mcts_agent.h"
#include "random_agent.h"

namespace pioneer {

std::unique_ptr<Agent> make_agent(const std::string& name, uint64_t seed) {
    if (name == "random") return std::make_unique<RandomAgent>(seed);
    if (name == "greedy") return std::make_unique<GreedyAgent>(seed);

    // "alphabeta" defaults to depth 4; "alphabeta-d6" names the depth,
    // which matters because search depth is part of an agent's identity
    // for rating purposes.
    if (name.rfind("alphabeta", 0) == 0) {
        AlphaBetaAgent::Options options;
        if (const size_t d = name.find("-d"); d != std::string::npos)
            options.depth = std::atoi(name.c_str() + d + 2);
        options.quiescence = name.find("-noqs") == std::string::npos;
        options.transposition = name.find("-nott") == std::string::npos;
        options.move_ordering = name.find("-noorder") == std::string::npos;
        // Opt-in, because measured against an outside engine these cost
        // more than they give; kept for tuning, not for playing.
        if (name.find("+pst") != std::string::npos) options.weights.placement = 100;
        if (name.find("+ks") != std::string::npos) options.weights.king_safety = 100;
        return std::make_unique<AlphaBetaAgent>(options, seed);
    }
    // "mcts" defaults to 1000 playouts; "mcts-s5000" names the budget,
    // which is part of the agent's identity for rating purposes.
    if (name.rfind("mcts", 0) == 0) {
        int simulations = 1000;
        if (const size_t s = name.find("-s"); s != std::string::npos)
            simulations = std::atoi(name.c_str() + s + 2);
        return std::make_unique<MctsAgent>(simulations, seed);
    }
    // "fsf" runs Fairy-Stockfish as an opponent; "fsf-d8" names the
    // depth. An external reference is what gives the internal ladder an
    // absolute position rather than just an order.
    if (name.rfind("fsf", 0) == 0) {
        int depth = 6;
        if (const size_t d = name.find("-d"); d != std::string::npos)
            depth = std::atoi(name.c_str() + d + 2);
        return std::make_unique<ExternalEngine>(fairy_stockfish_spec(depth));
    }
    return nullptr;
}

std::vector<std::string> agent_names() {
    return {"random", "greedy",
            "alphabeta[-dN][-noqs][-nott][-noorder][+pst][+ks]",
            "mcts[-sN]", "fsf[-dN] (external)"};
}

}  // namespace pioneer
