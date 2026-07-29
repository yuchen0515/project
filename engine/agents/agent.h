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

    // A one-call wall-clock override used by USI "go movetime". Agents
    // without time control keep their normal choose() behaviour. The
    // override is deliberately not persistent: a later bare "go" uses
    // the agent's configured mode again.
    virtual Move choose(const Game& game, int movetime_ms) {
        (void)movetime_ms;
        return choose(game);
    }

    // Called before each game. The seed comes from the driver and is
    // the only source of deliberate randomness an agent may use.
    // Fixed-work agents therefore replay move for move from the same
    // seeds. Wall-clock searches are inherently scheduling-dependent
    // and do not make that replay guarantee. Deterministic agents ignore
    // the seed.
    virtual void new_game(uint64_t seed) { (void)seed; }
};

}  // namespace pioneer
