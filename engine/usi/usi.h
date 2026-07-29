// USI (Universal Shogi Interface) front end.
//
// Commands are handled one line at a time against an output stream so
// the protocol can be tested directly, without spawning a process and
// racing on pipes.

#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <string>

#include "../agents/agent.h"

namespace pioneer {

constexpr const char* ENGINE_NAME = "Pioneer";
constexpr const char* ENGINE_AUTHOR = "Yu-Chen Lin";

class UsiEngine {
   public:
    explicit UsiEngine(std::unique_ptr<Agent> agent);

    // Returns false when the session should end (quit, or end of input).
    bool handle(const std::string& line, std::ostream& out);
    void loop(std::istream& in, std::ostream& out);

    const Game& game() const { return game_; }

   private:
    bool set_position(const std::string& args, std::ostream& out);
    void go(const std::string& args, std::ostream& out);

    Game game_;
    std::unique_ptr<Agent> agent_;
    // Successive games in one session differ. Fixed-work searches replay
    // the session identically; wall-clock "go movetime" searches cannot.
    uint64_t game_seed_ = 0;
};

}  // namespace pioneer
