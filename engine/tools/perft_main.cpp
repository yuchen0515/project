// perft [depth] [sfen] [--divide]
//
// Node counts per depth, and with --divide the per-move breakdown that
// makes a mismatch against another engine traceable to one move.

#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

#include "../core/movegen.h"

using namespace pioneer;

namespace {

void divide(const Position& pos, int depth) {
    MoveList moves;
    generate_legal(pos, moves);

    uint64_t total = 0;
    for (Move m : moves) {
        Position next = pos;
        next.apply(m);
        const uint64_t nodes = depth > 1 ? perft(next, depth - 1) : 1;
        total += nodes;
        std::printf("%-6s %llu\n", usi_move(m).c_str(),
                    static_cast<unsigned long long>(nodes));
    }
    std::printf("moves %d, nodes %llu\n", moves.size(),
                static_cast<unsigned long long>(total));
}

}  // namespace

int main(int argc, char** argv) {
    int max_depth = 5;
    std::string sfen = START_SFEN;
    bool want_divide = false;

    // An SFEN can start with a digit ("4k/5/..."), so a depth argument
    // is only one that is digits all the way through.
    auto all_digits = [](const char* s) {
        for (const char* p = s; *p; ++p)
            if (*p < '0' || *p > '9') return false;
        return *s != '\0';
    };

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--divide") == 0)
            want_divide = true;
        else if (all_digits(argv[i]))
            max_depth = std::atoi(argv[i]);
        else
            sfen = argv[i];
    }

    Position pos;
    if (!pos.set(sfen)) {
        std::fprintf(stderr, "illegal or malformed sfen: %s\n", sfen.c_str());
        return 1;
    }
    std::printf("%s\n%s\n", sfen.c_str(), pos.pretty().c_str());

    if (want_divide) {
        divide(pos, max_depth);
        return 0;
    }

    for (int depth = 1; depth <= max_depth; ++depth) {
        const auto start = std::chrono::steady_clock::now();
        const uint64_t nodes = perft(pos, depth);
        const double secs = std::chrono::duration<double>(
                                std::chrono::steady_clock::now() - start)
                                .count();
        std::printf("depth %d  %12llu  %7.2fs  %.0f nodes/s\n", depth,
                    static_cast<unsigned long long>(nodes), secs,
                    secs > 0 ? nodes / secs : 0.0);
        std::fflush(stdout);
    }
    return 0;
}
