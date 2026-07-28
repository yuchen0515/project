# Pioneer — a Minishogi (5五將棋) engine

Minishogi is shogi on a 5x5 board: six piece types, drops, promotion, and a
branching factor that makes it a genuinely hard game squeezed into 25 squares.
Pioneer is a minishogi program with an SDL2 GUI and an MCTS-based agent,
started as a 2020–2021 undergraduate capstone at NTNU CSIE (advisor:
Prof. Shun-Shii Lin) and fielded at the TCGA 2021 computer game tournament.

![Demo](./project-demo.gif)

## Current state

The original GUI is fully playable: human vs human, human vs agent, and
agent vs agent, with legal-move hints, drops from the captured-piece stand,
promotion, and game records written to disk for replay.

A 2026 audit of the legacy engine found fundamental defects in its search
layer (documented in [docs/ROADMAP.md](docs/ROADMAP.md)), so the engine core
is being rebuilt from the ground up: full rules coverage proven by perft and
tests, a USI-style protocol, stronger agents, and a rated self-play arena.
The roadmap describes the whole arc.

## Building

Dependencies: a C++17 compiler and SDL2.

```sh
# macOS
brew install sdl2

# Debian/Ubuntu
sudo apt install libsdl2-dev
```

```sh
cd code
make        # builds ./Pioneer
make run    # builds and starts the game
```

Developed and tested on macOS (Apple Silicon, Apple clang 17); Linux should
work with the same Makefile. The binary loads assets from `../image`, so
start it from the `code/` directory.

## Playing

On startup the program asks a few questions on stdin:

| Prompt | Meaning |
| --- | --- |
| `Your stepStory?` | Name of a previous game record to replay (without `.txt`), or `N` to start fresh |
| `Round` | Number of games to play in a row |
| `Simulation(Lower)` / `(Upper)` | MCTS simulation budget for each side's agent |
| `Type` | 1 human vs human, 2/3 agent on one side, 4 agent vs agent |

In game: left-click a piece to see its moves, click a highlighted square to
move; click a captured piece on the stand to drop it. Press `a` for a new
game, `Esc` to quit. Pieces promote on reaching the far rank. Game records
are written to the working directory as timestamped `.txt` files.

## Layout

```
code/            game + legacy engine (SDL2 GUI, MCTS agent)
  algorithm/     search, game state, record I/O
image/           board and piece bitmaps
docs/            roadmap and the original 2021 write-up (zh-TW)
```

## History

The original Chinese capstone report — including the threat-space MCTS idea
the project explored — is preserved verbatim in
[docs/report-2021_zh.md](docs/report-2021_zh.md), with slides
[here](https://reurl.cc/bnojyl).

## License

BSD 2-Clause. See [LICENSE](LICENSE).
