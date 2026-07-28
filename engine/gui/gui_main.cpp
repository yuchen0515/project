// A mouse-driven board for playing the engine.
//
// The original SDL front end drove its own board array and its own idea
// of the rules. This one owns nothing but pixels: Game decides what is
// legal, an Agent supplies the replies. It is a client of the engine,
// which is what makes it trustworthy to play against.

#include <SDL.h>

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "../agents/factory.h"
#include "../core/movegen.h"

using namespace pioneer;

namespace {

constexpr int SQUARE = 96;
constexpr int BOARD_X = 120;
constexpr int BOARD_Y = 150;
constexpr int BOARD_SIZE = SQUARE * FILE_NB;
constexpr int WINDOW_W = BOARD_X * 2 + BOARD_SIZE;
constexpr int WINDOW_H = BOARD_Y + BOARD_SIZE + 150;

// The piece bitmaps are a 375x667 canvas with the art in the top-left
// corner and black elsewhere. Drawing the whole canvas shrinks the koma
// to a quarter of its apparent size, which is what the original front
// end was compensating for with its oversized destination rects.
constexpr SDL_Rect PIECE_ART = {0, 0, 153, 172};
constexpr int PIECE_H = 86;
constexpr int PIECE_W = PIECE_H * PIECE_ART.w / PIECE_ART.h;

constexpr int HAND_SLOT = 56;
constexpr int HAND_TOP_Y = 40;                              // white's hand
constexpr int HAND_BOTTOM_Y = BOARD_Y + BOARD_SIZE + 20;    // black's hand

struct Selection {
    bool active = false;
    bool from_hand = false;
    Square square = SQ_NONE;
    PieceType hand_piece = NO_PIECE_TYPE;
    Bitboard targets = 0;
};

// color_key is for the piece art only: the board photograph has
// legitimately dark pixels that keying would punch holes in.
SDL_Texture* load(SDL_Renderer* renderer, const std::string& path,
                  bool color_key) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::fprintf(stderr, "cannot load %s: %s\n", path.c_str(),
                     SDL_GetError());
        return nullptr;
    }
    // Convert first, so the pixel below is read in a known layout
    // rather than whatever the file happened to use.
    SDL_Surface* rgba =
        SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surface);
    if (!rgba) return nullptr;

    // Key on the far corner, which is background in every one of these
    // files. Keying on the near corner would punch a hole in the art:
    // it starts at pixel (0, 0).
    if (color_key) {
        SDL_LockSurface(rgba);
        const uint32_t* pixels = static_cast<const uint32_t*>(rgba->pixels);
        const uint32_t background =
            pixels[(rgba->h - 1) * (rgba->pitch / 4) + (rgba->w - 1)];
        SDL_UnlockSurface(rgba);
        SDL_SetColorKey(rgba, SDL_TRUE, background);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rgba);
    SDL_FreeSurface(rgba);
    return texture;
}

struct Assets {
    SDL_Texture* board = nullptr;
    SDL_Texture* highlight = nullptr;
    SDL_Texture* piece[PIECE_TYPE_NB] = {};
    SDL_Texture* jade = nullptr;  // the other side's king

    bool load_all(SDL_Renderer* renderer, const std::string& dir) {
        board = load(renderer, dir + "board.bmp", false);
        highlight = load(renderer, dir + "latticecover.bmp", true);
        jade = load(renderer, dir + "jade.bmp", true);
        piece[KING] = load(renderer, dir + "king.bmp", true);
        piece[ROOK] = load(renderer, dir + "rook.bmp", true);
        piece[BISHOP] = load(renderer, dir + "bishop.bmp", true);
        piece[GOLD] = load(renderer, dir + "gold.bmp", true);
        piece[SILVER] = load(renderer, dir + "sliver.bmp", true);
        piece[PAWN] = load(renderer, dir + "pawn.bmp", true);
        piece[DRAGON] = load(renderer, dir + "rookup.bmp", true);
        piece[HORSE] = load(renderer, dir + "bishopup.bmp", true);
        piece[PRO_SILVER] = load(renderer, dir + "sliverup.bmp", true);
        piece[PRO_PAWN] = load(renderer, dir + "pawnup.bmp", true);
        return board && piece[KING] && piece[PAWN];
    }
};

SDL_Rect square_rect(Square s) {
    return {BOARD_X + col_of(s) * SQUARE, BOARD_Y + rank_of(s) * SQUARE, SQUARE,
            SQUARE};
}

SDL_Rect piece_rect_in(const SDL_Rect& cell) {
    return {cell.x + (cell.w - PIECE_W) / 2, cell.y + (cell.h - PIECE_H) / 2,
            PIECE_W, PIECE_H};
}

// The hand is laid out one slot per piece type, so a type keeps its
// place as the count changes.
SDL_Rect hand_rect(Color c, int index) {
    return {BOARD_X + index * HAND_SLOT,
            c == BLACK ? HAND_BOTTOM_Y : HAND_TOP_Y, HAND_SLOT, HAND_SLOT + 40};
}

void draw_piece(SDL_Renderer* renderer, const Assets& assets, Piece piece,
                const SDL_Rect& cell) {
    const PieceType type = type_of(piece);
    SDL_Texture* texture =
        (type == KING && color_of(piece) == WHITE) ? assets.jade
                                                   : assets.piece[type];
    if (!texture) return;

    const SDL_Rect dest = piece_rect_in(cell);
    // White's pieces face the other way, as they do on a real board.
    if (color_of(piece) == WHITE)
        SDL_RenderCopyEx(renderer, texture, &PIECE_ART, &dest, 180.0, nullptr,
                         SDL_FLIP_NONE);
    else
        SDL_RenderCopy(renderer, texture, &PIECE_ART, &dest);
}

Square square_at(int x, int y) {
    if (x < BOARD_X || y < BOARD_Y) return SQ_NONE;
    const int col = (x - BOARD_X) / SQUARE;
    const int rank = (y - BOARD_Y) / SQUARE;
    return is_square(rank, col) ? make_square(rank, col) : SQ_NONE;
}

Bitboard targets_from(const Game& game, Square from) {
    MoveList moves;
    generate_legal(game.position(), moves);
    Bitboard bb = 0;
    for (Move m : moves)
        if (!is_drop(m) && from_sq(m) == from) bb |= square_bb(to_sq(m));
    return bb;
}

Bitboard drop_targets(const Game& game, PieceType pt) {
    MoveList moves;
    generate_legal(game.position(), moves);
    Bitboard bb = 0;
    for (Move m : moves)
        if (is_drop(m) && drop_type(m) == pt) bb |= square_bb(to_sq(m));
    return bb;
}

void report(const Game& game) {
    if (!game.over()) return;
    const char* winner =
        game.outcome() == Outcome::BLACK_WIN ? "Black" : "White";
    std::printf("game over: %s wins by %s\n", winner, game.termination());
    std::fflush(stdout);
}

void draw(SDL_Renderer* renderer, const Assets& assets, const Game& game,
          const Selection& selection, Color human) {
    (void)human;
    SDL_SetRenderDrawColor(renderer, 38, 34, 31, 255);
    SDL_RenderClear(renderer);

    const SDL_Rect board_rect = {BOARD_X, BOARD_Y, BOARD_SIZE, BOARD_SIZE};
    SDL_RenderCopy(renderer, assets.board, nullptr, &board_rect);

    // Grid, so the squares read clearly whatever the board art does.
    SDL_SetRenderDrawColor(renderer, 40, 30, 20, 200);
    for (int i = 0; i <= FILE_NB; ++i) {
        SDL_RenderDrawLine(renderer, BOARD_X + i * SQUARE, BOARD_Y,
                           BOARD_X + i * SQUARE, BOARD_Y + BOARD_SIZE);
        SDL_RenderDrawLine(renderer, BOARD_X, BOARD_Y + i * SQUARE,
                           BOARD_X + BOARD_SIZE, BOARD_Y + i * SQUARE);
    }

    if (selection.active) {
        Bitboard targets = selection.targets;
        while (targets) {
            const SDL_Rect cell = square_rect(pop_lsb(targets));
            if (assets.highlight) {
                SDL_SetTextureAlphaMod(assets.highlight, 140);
                SDL_RenderCopy(renderer, assets.highlight, nullptr, &cell);
            } else {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 80, 140, 220, 120);
                SDL_RenderFillRect(renderer, &cell);
            }
        }
        if (!selection.from_hand) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 240, 200, 60, 110);
            const SDL_Rect cell = square_rect(selection.square);
            SDL_RenderFillRect(renderer, &cell);
        }
    }

    for (int s = 0; s < SQUARE_NB; ++s)
        if (const Piece piece = game.position().piece_on(Square(s));
            piece != NO_PIECE)
            draw_piece(renderer, assets, piece, square_rect(Square(s)));

    for (Color c : {BLACK, WHITE})
        for (int i = 0; i < HAND_TYPE_NB; ++i) {
            const PieceType pt = hand_type(i);
            const int count = game.position().hand_count(c, pt);
            if (!count) continue;
            const SDL_Rect slot = hand_rect(c, i);
            draw_piece(renderer, assets, make_piece(c, pt), slot);
            // A stack is shown as overlapping copies rather than a
            // number, since there is no text renderer here.
            for (int extra = 1; extra < count; ++extra) {
                SDL_Rect shifted = slot;
                shifted.x += 12 * extra;
                draw_piece(renderer, assets, make_piece(c, pt), shifted);
            }
        }

}

}  // namespace

int main(int argc, char** argv) {
    std::string agent_name = "alphabeta-d4";
    std::string image_dir = "../image/";
    std::string screenshot_path;
    std::string sfen = START_SFEN;
    Color human = BLACK;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--white")
            human = WHITE;
        else if (arg == "--images" && i + 1 < argc)
            image_dir = argv[++i];
        else if (arg == "--screenshot" && i + 1 < argc)
            screenshot_path = argv[++i];
        else if (arg == "--sfen" && i + 1 < argc)
            sfen = argv[++i];
        else if (arg.rfind("--", 0) == 0) {
            // Anything else starting with -- is a typo, not an agent
            // name; silently treating it as one hides the mistake.
            std::fprintf(stderr, "unknown option: %s\n", arg.c_str());
            return 1;
        } else
            agent_name = arg;
    }
    if (!image_dir.empty() && image_dir.back() != '/') image_dir += '/';

    auto agent = make_agent(agent_name);
    if (!agent) {
        std::fprintf(stderr, "unknown agent: %s\n", agent_name.c_str());
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window =
        SDL_CreateWindow("Pioneer - Minishogi", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H,
                         SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window || !renderer) {
        std::fprintf(stderr, "SDL window failed: %s\n", SDL_GetError());
        return 1;
    }

    Assets assets;
    if (!assets.load_all(renderer, image_dir)) {
        std::fprintf(stderr,
                     "could not load piece images from %s - pass --images "
                     "<dir> if you are running from elsewhere\n",
                     image_dir.c_str());
        return 1;
    }

    Game game;
    if (!game.set(sfen)) {
        std::fprintf(stderr, "illegal position: %s\n", sfen.c_str());
        return 1;
    }
    agent->new_game(1);
    Selection selection;

    // Renders one frame to a file and exits, so the layout can be
    // checked without a person having to look at a window.
    if (!screenshot_path.empty()) {
        draw(renderer, assets, game, selection, human);
        SDL_Surface* shot = SDL_CreateRGBSurfaceWithFormat(
            0, WINDOW_W, WINDOW_H, 32, SDL_PIXELFORMAT_ARGB8888);
        SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888,
                             shot->pixels, shot->pitch);
        SDL_SaveBMP(shot, screenshot_path.c_str());
        SDL_FreeSurface(shot);
        std::printf("wrote %s\n", screenshot_path.c_str());
        return 0;
    }

    std::printf(
        "You are %s. Click a piece, then a highlighted square.\n"
        "Promotion happens automatically when it is the only option, and by "
        "default when it is a choice - hold Shift while clicking to decline.\n"
        "Keys: n new game, u undo, Esc quit.\n",
        human == BLACK ? "Black (bottom)" : "White (top)");
    std::fflush(stdout);

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;

            if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) quit = true;
                if (key == SDLK_n) {
                    game.set(START_SFEN);
                    agent->new_game(uint64_t(SDL_GetTicks()));
                    selection = Selection{};
                }
                if (key == SDLK_u && game.ply() >= 2) {
                    // Rewind both plies by replaying the game without
                    // them - Game deliberately has no undo, since search
                    // copies positions rather than unmaking moves.
                    const std::vector<Move> played = game.moves();
                    game.set(game.initial_sfen());
                    for (size_t i = 0; i + 2 < played.size(); ++i)
                        game.apply(played[i]);
                    selection = Selection{};
                }
            }

            if (event.type != SDL_MOUSEBUTTONDOWN) continue;
            if (game.over() || game.side_to_move() != human) continue;

            if (event.button.button == SDL_BUTTON_RIGHT) {
                selection = Selection{};
                continue;
            }

            const int mx = event.button.x, my = event.button.y;
            const Square clicked = square_at(mx, my);

            // Completing a move.
            if (selection.active && clicked != SQ_NONE &&
                (selection.targets & square_bb(clicked))) {
                Move move = MOVE_NONE;
                if (selection.from_hand) {
                    move = make_drop(selection.hand_piece, clicked);
                } else {
                    const bool decline =
                        (SDL_GetModState() & KMOD_SHIFT) != 0;
                    const Move plain = make_move(selection.square, clicked);
                    const Move promo = make_move(selection.square, clicked, true);

                    MoveList legal;
                    generate_legal(game.position(), legal);
                    const bool can_plain = legal.contains(plain);
                    const bool can_promo = legal.contains(promo);
                    move = (can_promo && !(decline && can_plain)) ? promo : plain;
                }

                if (game.apply(move)) {
                    std::printf("you play %s\n", usi_move(move).c_str());
                    std::fflush(stdout);
                    report(game);
                }
                selection = Selection{};
                continue;
            }

            // Selecting a piece on the board.
            if (clicked != SQ_NONE) {
                const Piece piece = game.position().piece_on(clicked);
                if (piece != NO_PIECE && color_of(piece) == human) {
                    selection.active = true;
                    selection.from_hand = false;
                    selection.square = clicked;
                    selection.targets = targets_from(game, clicked);
                } else {
                    selection = Selection{};
                }
                continue;
            }

            // Selecting a piece in hand.
            selection = Selection{};
            for (int i = 0; i < HAND_TYPE_NB; ++i) {
                const SDL_Rect slot = hand_rect(human, i);
                const PieceType pt = hand_type(i);
                if (!game.position().hand_count(human, pt)) continue;
                if (mx < slot.x || mx >= slot.x + slot.w || my < slot.y ||
                    my >= slot.y + slot.h)
                    continue;
                selection.active = true;
                selection.from_hand = true;
                selection.hand_piece = pt;
                selection.targets = drop_targets(game, pt);
            }
        }

        if (!game.over() && game.side_to_move() != human) {
            const Move reply = agent->choose(game);
            if (reply == MOVE_NONE || !game.apply(reply)) {
                std::printf("%s has no move\n", agent->name());
            } else {
                std::printf("%s plays %s\n", agent->name(),
                            usi_move(reply).c_str());
            }
            std::fflush(stdout);
            report(game);
        }

        draw(renderer, assets, game, selection, human);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
