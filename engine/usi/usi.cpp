#include "usi.h"

#include <sstream>
#include <utility>

namespace pioneer {

UsiEngine::UsiEngine(std::unique_ptr<Agent> agent) : agent_(std::move(agent)) {}

bool UsiEngine::set_position(const std::string& args, std::ostream& out) {
    std::istringstream is(args);
    std::string token;
    is >> token;

    std::string sfen;
    if (token == "startpos") {
        sfen = START_SFEN;
    } else if (token == "sfen") {
        // An SFEN is four whitespace-separated fields; anything after
        // them belongs to the optional "moves" list.
        std::string field;
        for (int i = 0; i < 4 && is >> field; ++i) {
            if (field == "moves") {
                sfen.clear();
                break;
            }
            if (i) sfen += ' ';
            sfen += field;
        }
    } else {
        out << "info string unknown position type: " << token << std::endl;
        return false;
    }

    if (sfen.empty() || !game_.set(sfen)) {
        out << "info string illegal position" << std::endl;
        return false;
    }

    is >> token;  // "moves", or nothing
    if (token == "moves") {
        std::string usi;
        while (is >> usi) {
            if (!game_.apply_usi(usi)) {
                out << "info string illegal move: " << usi << std::endl;
                return false;
            }
        }
    }
    return true;
}

void UsiEngine::go(const std::string& args, std::ostream& out) {
    std::istringstream is(args);
    std::string token;
    is >> token;

    // "go perft N" is not part of USI, but every shogi and chess engine
    // that can be cross-checked supports it, so this one does too.
    if (token == "perft") {
        int depth = 1;
        is >> depth;
        out << perft(game_.position(), depth) << std::endl;
        return;
    }

    if (game_.over()) {
        out << "bestmove resign" << std::endl;
        return;
    }

    const Move move = agent_->choose(game_);
    out << "bestmove " << (move == MOVE_NONE ? "resign" : usi_move(move))
        << std::endl;
}

bool UsiEngine::handle(const std::string& line, std::ostream& out) {
    std::istringstream is(line);
    std::string command;
    is >> command;

    std::string rest;
    std::getline(is, rest);
    if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

    if (command.empty()) return true;

    if (command == "usi") {
        out << "id name " << ENGINE_NAME << '\n'
            << "id author " << ENGINE_AUTHOR << '\n'
            << "usiok" << std::endl;
    } else if (command == "isready") {
        out << "readyok" << std::endl;
    } else if (command == "usinewgame") {
        game_.set(START_SFEN);
        agent_->new_game(++game_seed_);
    } else if (command == "position") {
        set_position(rest, out);
    } else if (command == "go") {
        go(rest, out);
    } else if (command == "sfen") {
        out << game_.position().sfen() << std::endl;
    } else if (command == "moves") {
        MoveList list;
        generate_legal(game_.position(), list);
        for (Move m : list) out << usi_move(m) << ' ';
        out << std::endl;
    } else if (command == "d") {
        out << game_.position().pretty();
        if (game_.over())
            out << "game over: " << game_.termination() << std::endl;
    } else if (command == "setoption" || command == "gameover") {
        // Accepted and ignored: no options yet, and the driver's result
        // announcement tells the engine nothing it cannot see itself.
    } else if (command == "quit") {
        return false;
    } else {
        out << "info string unknown command: " << command << std::endl;
    }
    return true;
}

void UsiEngine::loop(std::istream& in, std::ostream& out) {
    std::string line;
    while (std::getline(in, line))
        if (!handle(line, out)) break;
}

}  // namespace pioneer
