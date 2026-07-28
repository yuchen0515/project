# Roadmap

Pioneer started life as a 2020–2021 undergraduate capstone and competed at
TCGA 2021. The revival effort keeps the original project playable while
rebuilding the engine on solid foundations. Phases land in order; each phase
is only declared done when it is verified (tests, perft counts, arena games).

## Phase 0 — Build fix and repository hygiene (done)

Modern toolchains could no longer build the project. Fixed the build, removed
dead prototype code and a checked-in binary, cleaned all compiler warnings,
rewrote the Makefile (incremental, `-Wall -Wextra`), restructured the docs.

## Phase 1 — Engine core rewrite (done)

An audit of the legacy engine found fundamental defects in the search
(moves silently rejected inside the tree because move legality depended on
GUI state; child nodes expanded with the parent position's move list), plus
missing rules. Rather than patch around them, the core was rebuilt:

- Self-contained engine core: bitboards with compile-time attack tables,
  position with SFEN I/O and incremental Zobrist hashing, full legal move
  generation, and a game layer holding the history that repetition needs.
- Every rule covered: optional promotion (compulsory only for a pawn on the
  last rank), nifu, the last-rank pawn drop ban, uchifuzume, and the
  fourth-repetition loss for the first player.
- Correctness cross-checked against an independent engine and reconciled
  exactly, not approximately — see `verification.md`.
- USI protocol layer plus an `Agent` interface, so GUIs, match runners and
  future agents all attach at the same seam.
- The legacy engine moved to `legacy/`, still buildable and playable, with
  its defects documented rather than quietly discarded.

## Phase 2 — Baseline agents and tactical search

- Reference agents: random, greedy-material, fixed-depth alpha-beta with a
  sane evaluation, and a correctly implemented UCT-MCTS.
- A df-pn mate solver as a tactical oracle - the proper realization of the
  original project's "threat space" idea.

## Phase 3 — Strength

Everything here runs on a laptop; no GPU cluster assumed.

- Main line: AlphaZero-style self-play training with a small network sized
  for CPU inference (the 5x5 board makes this tractable).
- Control line: alpha-beta with self-play-tuned evaluation - the approach
  behind the strongest known minishogi programs.
- Opening book generated from self-play statistics. Both lines fight in the
  arena; whichever wins carries the flag.

Full solving is off the table: minishogi's state space is estimated at
2.38 * 10^18 positions (arXiv:2604.06189), far beyond local storage even at
one bit per position. Solving subspaces (opening book, mate search cache) is
the practical substitute.

## Phase 4 — Arena and ratings

- Match runner over the USI layer with time control and adjudication.
- Append-only SQLite results store as the single source of truth.
- Ratings recomputed from full history (Bradley-Terry MLE, anchored to a
  frozen reference agent); SPRT gates for accepting engine changes.

## Phase 5 — Online platform

Web UI for play and replay, external agent submissions in a sandbox, and a
public rating ladder built on the Phase 4 arena.
