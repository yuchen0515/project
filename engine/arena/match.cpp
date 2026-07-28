#include "match.h"

#include "../core/movegen.h"

namespace pioneer {

const char* termination_name(Termination t) {
    switch (t) {
        case Termination::CHECKMATE:     return "checkmate";
        case Termination::STALEMATE:     return "stalemate";
        case Termination::REPETITION:    return "repetition";
        case Termination::PLY_LIMIT:     return "ply limit";
        case Termination::ILLEGAL_MOVE:  return "illegal move";
    }
    return "?";
}

namespace {

Termination classify(const Game& game) {
    const std::string reason = game.termination();
    if (reason == "checkmate") return Termination::CHECKMATE;
    if (reason == "stalemate") return Termination::STALEMATE;
    return Termination::REPETITION;
}

}  // namespace

GameRecord play_game(Agent& black, Agent& white, uint64_t seed, int max_plies) {
    Game game;
    black.new_game(seed);
    white.new_game(seed);

    GameRecord record;
    record.seed = seed;

    while (!game.over() && game.ply() < max_plies) {
        Agent& mover = game.side_to_move() == BLACK ? black : white;
        const Move move = mover.choose(game);

        // apply() validates, so an agent cannot corrupt the game by
        // returning nonsense - it just loses on the spot, and we say so.
        if (!game.apply(move)) {
            record.outcome = game.side_to_move() == BLACK ? Outcome::WHITE_WIN
                                                          : Outcome::BLACK_WIN;
            record.termination = Termination::ILLEGAL_MOVE;
            record.illegal_move = usi_move(move);
            record.plies = game.ply();
            record.final_sfen = game.position().sfen();
            return record;
        }
    }

    record.outcome = game.outcome();
    record.termination =
        game.over() ? classify(game) : Termination::PLY_LIMIT;
    record.plies = game.ply();
    record.final_sfen = game.position().sfen();
    return record;
}

MatchResult play_match(Agent& a, Agent& b, int games, uint64_t base_seed,
                       int max_plies) {
    MatchResult result;

    for (int i = 0; i < games; ++i) {
        const bool a_is_black = (i % 2 == 0);
        Agent& black = a_is_black ? a : b;
        Agent& white = a_is_black ? b : a;

        const GameRecord record =
            play_game(black, white, base_seed + uint64_t(i), max_plies);

        ++result.games;
        if (record.termination == Termination::ILLEGAL_MOVE) ++result.illegal;
        if (a_is_black) ++result.a_as_black_games;

        if (record.outcome == Outcome::ONGOING) {
            ++result.unfinished;
            continue;
        }

        const bool a_won = (record.outcome == Outcome::BLACK_WIN) == a_is_black;
        if (a_won) {
            ++result.wins_a;
            if (a_is_black) ++result.a_as_black_wins;
        } else {
            ++result.wins_b;
        }
    }

    return result;
}

}  // namespace pioneer
