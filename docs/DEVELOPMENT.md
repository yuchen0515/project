# Working on the engine

## Layout

```
engine/core/     rules: types, bitboards, position, move generation, game
engine/agents/   the Agent interface and everything that plays
engine/arena/    matches, ratings, and driving external engines
engine/usi/      USI protocol front end
engine/gui/      SDL board
engine/tools/    perft, arena, terminal play
engine/tests/    dependency-free test harness
legacy/          the 2021 program, frozen
```

`core/` owns the rules; nothing else is allowed an opinion about what is legal.
Front ends and agents are clients of `Game`, which is why a bug in a front end
cannot make the engine play an illegal move.

```sh
cd engine && make        # builds everything, then runs the tests
make build               # binaries only
make gui                 # SDL board (needs SDL2; outside `all` on purpose)
```

Binaries are built before tests run, deliberately. With tests as a
prerequisite, a failing test aborts the build and leaves stale binaries in
place — which quietly invalidates whatever you measure with them next.

## Adding an agent

Implement three methods and register it:

```cpp
class MyAgent : public Agent {
    const char* name() const override;      // must identify the configuration
    Move choose(const Game& game) override; // legal move, or MOVE_NONE
    void new_game(uint64_t seed) override;  // reset RNG and any state
};
```

`name()` carries the configuration, not just the family (`alphabeta-d4`,
`mcts-s2000`), because search depth and playout budget are part of an agent's
identity once ratings are involved. `new_game` receives the driver's seed and
must be the only source of randomness: a match has to replay move for move
from its seeds, which is what makes a reported result checkable.

Register it in `agents/factory.cpp` and it is immediately available to the
arena, both front ends and the USI binary.

## Measuring a change

```sh
tools/arena <new> <old> 80                   # cheap screen
tools/arena <new> fsf-d6 150 --seed 500      # the one that decides
tools/arena <old> fsf-d6 150 --seed 500      # same seed, same length
```

Three rules, each of which this project learned the hard way:

**Self-play is a screen, not a verdict.** Two agents sharing an evaluation
reward whichever one exploits the blind spots they have in common. A
positional evaluation here won self-play by +228 Elo and lost about 230
against an outside engine — see [agents.md](agents.md). Only the external
reference settles a strength claim.

**Run the control.** A number for the new version alone means nothing. Same
opponent, same seed, same number of games, both versions.

**Read the interval, not the score.** `tools/arena` reports a Wilson interval
mapped through the logistic curve. If it straddles zero, or overlaps the
control's, you have not shown anything yet. 150 games a side is the working
threshold for a decision; 40 is enough to notice something is broken.

Comparisons between fixed depths must use depths of the same parity. An
odd-depth search evaluates immediately after its own move and scores a tempo
it has not paid for, which is worth about a ply on its own.

## What the tests hold in place

The suite is not only about crashes. These are claims the repository makes,
and each one is pinned:

- perft counts, cross-checked against an independent engine, with the
  uchifuzume difference reconciled node for node ([verification.md](verification.md))
- a match replays exactly from its seeds
- `Position::set()` refuses positions that cannot occur in a game
- the evaluation is symmetric under the 180-degree mirror
- an agent returning an illegal move forfeits and is counted, not tolerated
- positional evaluation terms stay off until they are tuned against something
  external

If you add behaviour, add the test that keeps it true. Half the value of this
repository is that what it claims is checked.
