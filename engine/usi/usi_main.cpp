#include <iostream>
#include <memory>
#include <string>

#include "../agents/factory.h"
#include "usi.h"

// pioneer [agent-name]
//
// Defaults to the strongest agent available rather than the baseline:
// anything driving this over USI wants a player, not a control.
int main(int argc, char** argv) {
    // USI is a line protocol over pipes; without this a GUI waiting on
    // our reply deadlocks against our own buffer.
    std::ios::sync_with_stdio(false);

    const std::string agent_name = argc > 1 ? argv[1] : "alphabeta-d4";
    auto agent = pioneer::make_agent(agent_name);
    if (!agent) {
        std::cerr << "unknown agent: " << agent_name << "\nagents:";
        for (const std::string& name : pioneer::agent_names())
            std::cerr << ' ' << name;
        std::cerr << std::endl;
        return 1;
    }

    pioneer::UsiEngine engine(std::move(agent));
    engine.loop(std::cin, std::cout);
    return 0;
}
