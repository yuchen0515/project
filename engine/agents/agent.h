// The interface every playing agent implements. The protocol layer and
// the arena drive agents through this and nothing else, so a new agent
// needs no changes anywhere outside its own file.

#pragma once

#include "../core/game.h"

namespace pioneer {

class Agent {
   public:
    virtual ~Agent() = default;

    // Reported over USI and recorded in match results, so it must
    // identify the agent and its configuration well enough to tell two
    // versions apart.
    virtual const char* name() const = 0;

    // Returns a legal move for game.position(), or MOVE_NONE when there
    // is none (the caller resigns on its behalf). Agents may assume the
    // game is not already over.
    virtual Move choose(const Game& game) = 0;

    // Called before each new game. Agents holding search state or an
    // RNG stream reset it here; a fixed seed keeps matches reproducible.
    virtual void new_game() {}
};

}  // namespace pioneer
