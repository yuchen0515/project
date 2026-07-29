// Drives another USI engine as if it were one of our agents, so the
// internal ladder can be anchored to something outside this repository.
//
// Ratings measured only against your own agents say how the ladder is
// ordered, never where it sits. An external reference fixes that.

#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "../agents/agent.h"

namespace pioneer {

struct ExternalEngineSpec {
    std::string label;                  // agent name, e.g. "fsf-d6"
    std::string command;                // binary to run
    std::vector<std::string> args;
    // Sent before the usi handshake, and after it: engines differ on
    // which options survive the handshake. Fairy-Stockfish needs its
    // protocol set before and its variant after.
    std::vector<std::string> setup_before_handshake;
    std::vector<std::string> setup_after_handshake;
    // Fixed depth keeps matches reproducible; a movetime does not, and
    // is only for measuring against a wall clock deliberately.
    int depth = 0;
    int movetime_ms = 0;
};

// Fairy-Stockfish 14.x playing minishogi at a fixed depth.
ExternalEngineSpec fairy_stockfish_spec(int depth);
// Fairy-Stockfish 14.x playing minishogi for a fixed time per move.
ExternalEngineSpec fairy_stockfish_movetime_spec(int movetime_ms);

class ExternalEngine : public Agent {
   public:
    explicit ExternalEngine(ExternalEngineSpec spec);
    ~ExternalEngine() override;

    ExternalEngine(const ExternalEngine&) = delete;
    ExternalEngine& operator=(const ExternalEngine&) = delete;

    const char* name() const override { return spec_.label.c_str(); }
    Move choose(const Game& game) override;
    void new_game(uint64_t seed) override;

    bool running() const { return pid_ > 0; }
    // Empty unless the process died or misbehaved.
    const std::string& error() const { return error_; }

   private:
    void send(const std::string& line);
    // Reads until a line starting with prefix, or the engine dies.
    std::string read_until(const std::string& prefix);
    void shutdown();

    ExternalEngineSpec spec_;
    pid_t pid_ = -1;
    int to_engine_ = -1;
    std::FILE* from_engine_ = nullptr;
    std::string error_;
};

}  // namespace pioneer
