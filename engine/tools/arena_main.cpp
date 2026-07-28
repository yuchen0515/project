// arena <agent-a> <agent-b> [games] [--seed N] [--max-plies N] [--games-detail]
//
// Plays a match with alternating colours and reports the score with the
// interval around it, because a raw win count is not evidence on its own.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "../agents/factory.h"
#include "../arena/match.h"
#include "../arena/rating.h"

using namespace pioneer;

namespace {

void usage() {
    std::fprintf(stderr, "usage: arena <agent-a> <agent-b> [games]"
                         " [--seed N] [--max-plies N] [--games-detail]\n"
                         "agents:");
    for (const std::string& name : agent_names())
        std::fprintf(stderr, " %s", name.c_str());
    std::fprintf(stderr, "\n");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        usage();
        return 1;
    }

    const std::string name_a = argv[1];
    const std::string name_b = argv[2];
    int games = 100;
    uint64_t seed = 1;
    int max_plies = DEFAULT_MAX_PLIES;
    bool detail = false;

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--seed" && i + 1 < argc)
            seed = std::strtoull(argv[++i], nullptr, 10);
        else if (arg == "--max-plies" && i + 1 < argc)
            max_plies = std::atoi(argv[++i]);
        else if (arg == "--games-detail")
            detail = true;
        else
            games = std::atoi(arg.c_str());
    }

    auto a = make_agent(name_a, 1);
    auto b = make_agent(name_b, 2);
    if (!a || !b) {
        usage();
        return 1;
    }

    std::map<std::string, int> terminations;
    long long total_plies = 0;

    MatchResult result;
    for (int i = 0; i < games; ++i) {
        const bool a_is_black = (i % 2 == 0);
        const GameRecord record =
            play_game(a_is_black ? *a : *b, a_is_black ? *b : *a,
                      seed + uint64_t(i), max_plies);

        ++terminations[termination_name(record.termination)];
        total_plies += record.plies;

        ++result.games;
        if (record.termination == Termination::ILLEGAL_MOVE) ++result.illegal;
        if (a_is_black) ++result.a_as_black_games;
        if (record.outcome == Outcome::ONGOING) {
            ++result.unfinished;
        } else if ((record.outcome == Outcome::BLACK_WIN) == a_is_black) {
            ++result.wins_a;
            if (a_is_black) ++result.a_as_black_wins;
        } else {
            ++result.wins_b;
        }

        if (detail)
            std::printf("game %-4d seed %-6llu %-12s %4d plies  %s\n", i + 1,
                        (unsigned long long)record.seed,
                        termination_name(record.termination), record.plies,
                        record.final_sfen.c_str());
    }

    std::printf("%s vs %s: %d-%d", name_a.c_str(), name_b.c_str(),
                result.wins_a, result.wins_b);
    if (result.unfinished) std::printf(" (%d unfinished)", result.unfinished);
    std::printf("  over %d games\n", result.games);

    const double score = result.score_a();
    std::printf("score  %.1f%%  (%d decisive)\n", 100.0 * score,
                result.decisive());

    const EloEstimate elo = elo_estimate(score, result.decisive());
    if (elo.finite)
        std::printf("elo    %+.0f  [%+.0f, %+.0f]  95%% confidence\n", elo.elo,
                    elo.lower, elo.upper);
    else
        std::printf("elo    beyond measurement at this sample size"
                    " (one side won every decisive game); %s bound %+.0f\n",
                    score >= 1.0 ? "lower" : "upper",
                    score >= 1.0 ? elo.lower : elo.upper);

    if (result.a_as_black_games) {
        const int as_white_games = result.games - result.a_as_black_games;
        const int as_white_wins = result.wins_a - result.a_as_black_wins;
        std::printf("colours %s won %d/%d as black, %d/%d as white\n",
                    name_a.c_str(), result.a_as_black_wins,
                    result.a_as_black_games, as_white_wins, as_white_games);
    }

    std::printf("length %.1f plies average\n",
                result.games ? double(total_plies) / result.games : 0.0);
    std::printf("ended  ");
    for (const auto& [reason, count] : terminations)
        std::printf("%s %d  ", reason.c_str(), count);
    std::printf("\n");

    if (result.illegal)
        std::fprintf(stderr,
                     "warning: %d game(s) ended on an illegal move - that is"
                     " an agent bug, not a result\n",
                     result.illegal);
    return 0;
}
