#include "factory.h"

#include "greedy_agent.h"
#include "random_agent.h"

namespace pioneer {

std::unique_ptr<Agent> make_agent(const std::string& name, uint64_t seed) {
    if (name == "random") return std::make_unique<RandomAgent>(seed);
    if (name == "greedy") return std::make_unique<GreedyAgent>(seed);
    return nullptr;
}

std::vector<std::string> agent_names() { return {"random", "greedy"}; }

}  // namespace pioneer
