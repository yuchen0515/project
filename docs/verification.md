# Verifying move generation

An engine that generates moves slightly wrongly still plays chess-looking
games; the bug only shows up as inexplicable losses. So the move generator
is checked against an independent implementation before anything is built
on top of it.

## Perft

`perft(n)` counts the distinct legal move sequences of length `n`. It
exercises every rule at once — a single missing or extra move changes the
count — and any two correct implementations must agree exactly.

```sh
cd engine && make
tools/perft 6                            # start position, depths 1..6
tools/perft 5 "4k/5/5/5/K4 b RBGSP 1"    # any SFEN
tools/perft 3 --divide                   # per-move breakdown
```

`--divide` prints the subtree size of each root move, which is how a
disagreement gets narrowed down to a single move rather than argued about.

The values in `tests/test_perft.cpp` run as part of `make test` (about
three seconds) and fail loudly if generation ever changes.

## Cross-check against Fairy-Stockfish

[Fairy-Stockfish](https://fairy-stockfish.github.io/) 14.0.1 plays minishogi
and is entirely independent of this code.

```sh
brew install fairy-stockfish
printf 'setoption name UCI_Variant value minishogi\nposition startpos\ngo perft 6\nquit\n' \
  | fairy-stockfish | grep Nodes
```

| depth | Pioneer | Fairy-Stockfish |
| ---: | ---: | ---: |
| 1 | 14 | 14 |
| 2 | 181 | 181 |
| 3 | 2,512 | 2,512 |
| 4 | 35,401 | 35,401 |
| 5 | 533,203 | 533,203 |
| 6 | 8,276,188 | 8,276,188 |
| 7 | 132,680,617 | 132,680,698 |

Depth 5 also matches the value in Fairy-Stockfish's own test suite
(`tests/perft.sh`).

## The depth-7 difference is uchifuzume

The rules forbid delivering mate by dropping a pawn (uchifuzume): mate by
dropping any other piece is legal, and so is mate by a pawn already on the
board. Fairy-Stockfish's minishogi does not enforce this — it generates such
drops while agreeing the resulting position is mate (`go perft 1` there
returns 0).

That accounts for the gap exactly. A forbidden drop played before the last
ply leaves a mated position with no successors, contributing no leaves; only
drops available at the final ply change the count. Counting them directly:

```
pawn-drop mates available at ply 5: 0
pawn-drop mates available at ply 6: 0
pawn-drop mates available at ply 7: 81
```

and 132,680,617 + 81 = 132,680,698. The same reconciliation holds on
drop-heavy positions (9,635,599 + 148 = 9,635,747), and both are asserted in
`tests/test_perft.cpp`.

So the two engines agree on every rule they both implement, and the one rule
they disagree on is one the rules of minishogi settle in our favour.

## Repetition

The fourth occurrence of a position — same board, same hands, same side to
move — ends the game with the player who moved first losing. Minishogi
differs from shogi here, where sennichite is a drawn game to be replayed.
Sources agree on the rule
([Wikipedia](https://en.wikipedia.org/wiki/Minishogi),
[PyChess](https://www.pychess.org/variants/minishogi),
[H.G. Muller's rules page](http://hgm.nubati.net/rules/Mini.html)).

They differ on one edge: Muller states perpetual check is separately
forbidden, which in shogi means the checking side loses regardless of who
moved first. `Game` implements the majority rule (first player loses on the
fourth repetition) and does not special-case perpetual check; if a
tournament requires the other reading, it is a branch in `Game::outcome()`.

Position identity comes from the Zobrist key, which already covers hands and
side to move, so the comparison is a single 64-bit equality.

## Position validation

`Position::set()` rejects positions that cannot occur in a game, rather than
letting them fail deep inside a search:

- either king missing, or a king promoted
- more than two of a piece type, counting both colors, both hands, and
  promoted pieces as their base type
- the side that just moved leaving its own king capturable

Both of the last two were added after carelessly constructed test positions
tripped assertions inside `apply()` — the boundary is the right place to
catch them.
