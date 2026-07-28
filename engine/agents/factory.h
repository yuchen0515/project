// Builds agents by name, so tools take agents as arguments rather than
// hard-coding which ones exist.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "agent.h"

namespace pioneer {

// Returns nullptr for an unknown name.
std::unique_ptr<Agent> make_agent(const std::string& name, uint64_t seed = 0);

std::vector<std::string> agent_names();

}  // namespace pioneer
