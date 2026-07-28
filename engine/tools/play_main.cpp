// play [agent] [--white] [--sfen "<sfen>"]
//
// A board, a prompt, and an opponent. The USI binary is for programs;
// this is for people.

#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "../agents/factory.h"
#include "../core/movegen.h"

using namespace pioneer;

namespace {

const char* PIECE_NAME[PIECE_TYPE_NB] = {
    "",     "pawn",        "silver", "bishop", "rook",  "gold",
    "king", "+pawn",       "+silver", "horse",  "dragon"};

void print_board(const Game& game) {
    const Position& pos = game.position();

    std::printf("\n     5   4   3   2   1\n");
    std::printf("   +---+---+---+---+---+\n");
    for (int rank = 0; rank < RANK_NB; ++rank) {
        std::printf(" %c |", char('a' + rank));
        for (int col = 0; col < FILE_NB; ++col) {
            const Piece piece = pos.piece_on(make_square(rank, col));
            if (piece == NO_PIECE) {
                std::printf("   |");
                continue;
            }
            const PieceType type = type_of(piece);
            const char letter = PIECE_CHAR[unpromoted(type)];
            // Upper case for Black (you, by default), lower for White,
            // with + marking a promoted piece.
            std::printf("%c%c |", is_promoted(type) ? '+' : ' ',
                        color_of(piece) == BLACK
                            ? letter
                            : char(std::tolower(letter)));
        }
        std::printf(" %c\n   +---+---+---+---+---+\n", char('a' + rank));
    }
    std::printf("     5   4   3   2   1\n\n");

    for (Color c : {BLACK, WHITE}) {
        std::printf("%s in hand:", c == BLACK ? "BLACK (upper)" : "white (lower)");
        bool any = false;
        for (int i = 0; i < HAND_TYPE_NB; ++i) {
            const PieceType pt = hand_type(i);
            const int n = pos.hand_count(c, pt);
            if (!n) continue;
            any = true;
            std::printf(" %c", PIECE_CHAR[pt]);
            if (n > 1) std::printf("x%d", n);
        }
        std::printf("%s\n", any ? "" : " -");
    }
}

void print_legal_moves(const Game& game) {
    MoveList moves;
    generate_legal(game.position(), moves);

    // Grouped by origin, which is how a person looks for a move.
    std::map<std::string, std::set<std::string>> by_source;
    for (Move m : moves) {
        const std::string source =
            is_drop(m) ? std::string("drop ") + PIECE_CHAR[drop_type(m)]
                       : usi_square(from_sq(m)) + " " +
                             PIECE_NAME[type_of(game.position().piece_on(from_sq(m)))];
        by_source[source].insert(usi_move(m));
    }

    std::printf("%d legal moves:\n", moves.size());
    for (const auto& [source, targets] : by_source) {
        std::printf("  %-12s", source.c_str());
        for (const std::string& move : targets) std::printf(" %s", move.c_str());
        std::printf("\n");
    }
}

void print_help() {
    std::printf(
        "\nEnter moves in USI notation:\n"
        "  5d5c    move the piece on 5d to 5c\n"
        "  2b2a+   move and promote\n"
        "  P*3c    drop a pawn on 3c\n"
        "Files are 5..1 left to right, ranks a..e top to bottom.\n"
        "\nCommands: moves (list them)  board  help  quit\n\n");
}

void announce(const Game& game, Color human) {
    const Outcome outcome = game.outcome();
    const bool human_won = (outcome == Outcome::BLACK_WIN) == (human == BLACK);
    std::printf("\n%s by %s.\n", human_won ? "You win" : "You lose",
                game.termination());
}

}  // namespace

int main(int argc, char** argv) {
    std::string agent_name = "alphabeta-d4";
    std::string sfen = START_SFEN;
    Color human = BLACK;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--white")
            human = WHITE;
        else if (arg == "--sfen" && i + 1 < argc)
            sfen = argv[++i];
        else if (arg == "--help") {
            std::printf("usage: play [agent] [--white] [--sfen \"<sfen>\"]\n"
                        "agents:");
            for (const std::string& name : agent_names())
                std::printf(" %s", name.c_str());
            std::printf("\n");
            return 0;
        } else {
            agent_name = arg;
        }
    }

    auto agent = make_agent(agent_name);
    if (!agent) {
        std::fprintf(stderr, "unknown agent: %s\n", agent_name.c_str());
        return 1;
    }

    Game game;
    if (!game.set(sfen)) {
        std::fprintf(stderr, "illegal position: %s\n", sfen.c_str());
        return 1;
    }
    agent->new_game(1);

    std::printf("Minishogi - you are %s, the opponent is %s\n",
                human == BLACK ? "BLACK (upper case, moves up the board)"
                               : "white (lower case, moves down the board)",
                agent->name());
    print_help();

    while (!game.over()) {
        print_board(game);

        if (game.side_to_move() != human) {
            const Move reply = agent->choose(game);
            if (reply == MOVE_NONE || !game.apply(reply)) break;
            std::printf("%s plays %s\n", agent->name(), usi_move(reply).c_str());
            continue;
        }

        if (game.position().in_check()) std::printf("You are in check.\n");

        std::printf("your move> ");
        std::fflush(stdout);

        std::string input;
        if (!(std::cin >> input)) {
            std::printf("\n");
            return 0;
        }

        if (input == "quit" || input == "q") return 0;
        if (input == "help" || input == "?") {
            print_help();
            continue;
        }
        if (input == "moves") {
            print_legal_moves(game);
            continue;
        }
        if (input == "board") continue;

        const Move move = parse_usi_move(input);
        if (move == MOVE_NONE) {
            std::printf("Not a move I understand. Type help, or moves.\n");
            continue;
        }
        if (!game.apply(move)) {
            std::printf("%s is not legal here. Type moves to see what is.\n",
                        input.c_str());
            continue;
        }
    }

    print_board(game);
    announce(game, human);
    return 0;
}
