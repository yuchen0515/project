#include "external_engine.h"

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#include "../core/movegen.h"

namespace pioneer {

ExternalEngineSpec fairy_stockfish_spec(int depth) {
    ExternalEngineSpec spec;
    spec.label = "fsf-d" + std::to_string(depth);
    spec.command = "fairy-stockfish";
    // Fairy-Stockfish speaks UCI by default and only accepts USI square
    // names once its protocol is switched; the handshake then resets the
    // variant to full shogi, so minishogi has to be selected after it.
    spec.setup_before_handshake = {"setoption name Protocol value usi"};
    spec.setup_after_handshake = {
        "setoption name UCI_Variant value minishogi",
        "setoption name Threads value 1",  // one thread, or it is not reproducible
    };
    spec.depth = depth;
    return spec;
}

ExternalEngine::ExternalEngine(ExternalEngineSpec spec) : spec_(std::move(spec)) {
    // Writing to an engine that has died raises SIGPIPE, which by
    // default kills us along with it. Ignoring it turns that into an
    // EPIPE we can report - a dead opponent should forfeit, not take
    // the arena down.
    static const bool ignored = [] {
        signal(SIGPIPE, SIG_IGN);
        return true;
    }();
    (void)ignored;

    int to_child[2], from_child[2];
    if (pipe(to_child) != 0 || pipe(from_child) != 0) {
        error_ = "could not create pipes";
        return;
    }

    const pid_t pid = fork();
    if (pid < 0) {
        error_ = "fork failed";
        return;
    }

    if (pid == 0) {
        dup2(to_child[0], STDIN_FILENO);
        dup2(from_child[1], STDOUT_FILENO);
        close(to_child[0]);
        close(to_child[1]);
        close(from_child[0]);
        close(from_child[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(spec_.command.c_str()));
        for (std::string& arg : spec_.args) argv.push_back(&arg[0]);
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);  // only reached if exec failed
    }

    close(to_child[0]);
    close(from_child[1]);
    pid_ = pid;
    to_engine_ = to_child[1];
    from_engine_ = fdopen(from_child[0], "r");

    for (const std::string& line : spec_.setup_before_handshake) send(line);
    send("usi");
    if (read_until("usiok").empty()) {
        error_ = "no usiok from " + spec_.command;
        shutdown();
        return;
    }
    for (const std::string& line : spec_.setup_after_handshake) send(line);
    send("isready");
    if (read_until("readyok").empty()) {
        error_ = "no readyok from " + spec_.command;
        shutdown();
    }
}

ExternalEngine::~ExternalEngine() { shutdown(); }

void ExternalEngine::shutdown() {
    if (pid_ <= 0) return;

    send("quit");
    if (to_engine_ >= 0) {
        close(to_engine_);
        to_engine_ = -1;
    }

    // Give it a moment to leave on its own before insisting.
    for (int i = 0; i < 100; ++i) {
        int status = 0;
        const pid_t done = waitpid(pid_, &status, WNOHANG);
        if (done == pid_) {
            pid_ = -1;
            break;
        }
        usleep(10000);
    }
    if (pid_ > 0) {
        kill(pid_, SIGKILL);
        waitpid(pid_, nullptr, 0);
        pid_ = -1;
    }

    if (from_engine_) {
        std::fclose(from_engine_);
        from_engine_ = nullptr;
    }
}

void ExternalEngine::send(const std::string& line) {
    if (to_engine_ < 0) return;
    const std::string payload = line + "\n";
    if (write(to_engine_, payload.data(), payload.size()) < 0)
        error_ = "write to engine failed";
}

std::string ExternalEngine::read_until(const std::string& prefix) {
    if (!from_engine_) return "";

    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), from_engine_)) {
        std::string line(buffer);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.pop_back();
        if (line.rfind(prefix, 0) == 0) return line;
    }
    return "";  // pipe closed: the engine is gone
}

void ExternalEngine::new_game(uint64_t) {
    if (pid_ > 0) send("usinewgame");
}

Move ExternalEngine::choose(const Game& game) {
    if (pid_ <= 0) return MOVE_NONE;

    // Send the whole history, not just the current position, so the
    // engine can see repetitions coming.
    std::string command = "position sfen " + game.initial_sfen();
    if (!game.moves().empty()) {
        command += " moves";
        for (Move m : game.moves()) command += ' ' + usi_move(m);
    }
    send(command);

    if (spec_.movetime_ms > 0)
        send("go movetime " + std::to_string(spec_.movetime_ms));
    else
        send("go depth " + std::to_string(spec_.depth > 0 ? spec_.depth : 6));

    const std::string reply = read_until("bestmove");
    if (reply.empty()) {
        error_ = spec_.command + " stopped responding";
        return MOVE_NONE;
    }

    const size_t space = reply.find(' ');
    if (space == std::string::npos) return MOVE_NONE;
    const std::string token = reply.substr(space + 1, reply.find(' ', space + 1) -
                                                          space - 1);
    if (token == "resign" || token == "none") return MOVE_NONE;

    return parse_usi_move(token);
}

}  // namespace pioneer
