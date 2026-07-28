# Agents and what they are worth

Every number here came from `tools/arena`, which alternates colours, seeds
each game so the whole match replays from one number, and reports a Wilson
interval rather than a bare win count.

```sh
cd engine && make
tools/arena alphabeta-d3 greedy 200
tools/arena mcts-s1600 alphabeta-d3 60 --games-detail
```

## The ladder

| Agent | What it does |
| --- | --- |
| `random` | Uniformly random legal move. The zero point. |
| `greedy` | One ply: best material after its own move, mate if one is on the board. |
| `alphabeta[-dN][-noqs]` | Negamax with alpha-beta, MVV-LVA ordering and quiescence. Depth 4 by default. |
| `mcts[-sN]` | UCT with random playouts cut off at 24 plies. 1000 playouts by default. |

Measured over 100 games each unless noted, alternating colours:

| Match | Result | Elo |
| --- | --- | --- |
| `greedy` vs `random` | 196–4 (200 games) | +676 [+510, +842] |
| `mcts-s400` vs `random` | 99–1 | +798 [+496, +1101] |
| `mcts-s400` vs `greedy` | 86–14 | +315 [+218, +412] |
| `alphabeta-d2` vs `greedy` | 100–0 | > +566 |
| `alphabeta-d3` vs `mcts-s400` | 99–1 | +798 [+496, +1101] |
| `alphabeta-d3` vs `mcts-s1600` | 60–0 (60 games) | > +477 |

## Where that ladder actually sits

Ratings measured only against your own agents say how they are ordered,
never where they stand. [Fairy-Stockfish](https://fairy-stockfish.github.io/)
14.0.1 plays minishogi, speaks USI, and shares no code with this, so the
arena drives it as an ordinary agent:

```sh
brew install fairy-stockfish
tools/arena alphabeta-d4 fsf-d6 40
```

| Match | Result | Elo |
| --- | --- | --- |
| `alphabeta-d4` vs `fsf-d2` | 17-23 | -53 [-160, +55] |
| `alphabeta-d4` vs `fsf-d4` | 13-27 | -127 [-240, -14] |
| `alphabeta-d4` vs `fsf-d6` | 6-34 | -301 [-448, -155] |

So the engine as it stands is about level with Fairy-Stockfish searching two
plies, and roughly 300 Elo behind it at six. Depths are not comparable
between engines - theirs includes extensions and reductions ours does not
have - but the ranking is the point, and now every future change can be
measured against something outside this repository rather than only against
its own past.

Two caveats worth stating. Fairy-Stockfish does not enforce uchifuzume (see
verification.md), so it may offer a pawn-drop mate that our rules refuse; the
arena scores that as a forfeit and counts it separately, and it has not
happened in the matches above. And a fixed depth keeps the match
reproducible, where a time limit would not.

## What the numbers say

**Search depth is worth about 370 Elo per ply** here (`d3` beats `d2` 89–11,
`d4` beats `d3` 90–10). Extra budget helping is the result that proves a
search is really searching — it is exactly what the legacy engine could not
have produced, since its tree never advanced the position at all.

**Quiescence is worth roughly a ply at a fifth of the cost.** At equal depth
it gains +263 to +363 Elo. Against a full extra ply it is statistically level
(−42 Elo, interval [−110, +26]) while spending 2.6s where the deeper search
spends 13.8s over the same 20 games. Per unit of time it is clearly the
better use of the budget.

**Alpha-beta beats plain MCTS badly, and that is not a bug.** A shallow
material search takes 99 of 100 games from `mcts-s400`, and 60 of 60 even
when MCTS gets four times the playouts. Random playouts are close to
worthless in a game with drops: a captured piece re-enters anywhere, so a
random continuation from a sharp position tells you almost nothing about it,
and the tree averages noise. This is the same reason the strongest known
minishogi program, [Shokidoki](https://home.hccnet.nl/h.g.muller/shokidoki.html),
is alpha-beta rather than MCTS.

It is also the argument for phase 3. MCTS is not the wrong algorithm; random
rollouts are the wrong value estimate. Replacing them with a learned value —
the AlphaZero shape — is what would make the tree pay off, and if that lands,
`alphabeta_outplays_mcts_in_this_game` in the test suite is the assertion
that should flip.

## First-player advantage

Between two random agents, Black wins about 58% of decisive games (112 of
192 in a 200-game match). Minishogi favours the first player, and it shows
even between agents that understand nothing about the game — despite the
repetition rule, which scores the fourth occurrence of a position as a loss
for Black specifically.

## Reproducing any of this

Every match is a pure function of its agents and base seed:

```sh
tools/arena alphabeta-d3 mcts-s400 100 --seed 1
```

The same command gives the same games and the same result on any machine.
