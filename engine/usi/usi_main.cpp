#include <iostream>
#include <memory>

#include "../agents/random_agent.h"
#include "usi.h"

int main() {
    // USI is a line protocol over pipes; without this a GUI waiting on
    // our reply deadlocks against our own buffer.
    std::ios::sync_with_stdio(false);

    pioneer::UsiEngine engine(std::make_unique<pioneer::RandomAgent>());
    engine.loop(std::cin, std::cout);
    return 0;
}
