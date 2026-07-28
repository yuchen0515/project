// Playing agents against each other, reproducibly.

#pragma once

#include <string>

#include "../agents/agent.h"

namespace pioneer {

// Games that reach this many plies are reported unfinished rather than
// scored. The repetition rule ends most games long before it; a cap
// still matters because a shuffling agent can avoid repeating for a
// very long time.
constexpr int DEFAULT_MAX_PLIES = 400;

enum class Termination { CHECKMATE, STALEMATE, REPETITION, PLY_LIMIT, ILLEGAL_MOVE };

struct GameRecord {
    Outcome outcome = Outcome::ONGOING;
    Termination termination = Termination::PLY_LIMIT;
    int plies = 0;
    uint64_t seed = 0;
    std::string final_sfen;
    // Set when an agent returned a move that was not legal, which is a
    // bug in that agent rather than a result: it forfeits, loudly.
    std::string illegal_move;
};

const char* termination_name(Termination t);

GameRecord play_game(Agent& black, Agent& white, uint64_t seed,
                     int max_plies = DEFAULT_MAX_PLIES);

struct MatchResult {
    int games = 0;
    int wins_a = 0;
    int wins_b = 0;
    int unfinished = 0;
    int illegal = 0;

    // Colour split, because an agent that is strong only as Black is
    // telling you something about the agent or about the game.
    int a_as_black_games = 0;
    int a_as_black_wins = 0;

    int decisive() const { return wins_a + wins_b; }
    // Share of decisive games won by A, in [0, 1].
    double score_a() const {
        return decisive() ? double(wins_a) / decisive() : 0.5;
    }
};

// Colours alternate game by game, and each game gets its own seed
// derived from base_seed, so the whole match replays from that one
// number.
MatchResult play_match(Agent& a, Agent& b, int games, uint64_t base_seed = 1,
                       int max_plies = DEFAULT_MAX_PLIES);

}  // namespace pioneer
