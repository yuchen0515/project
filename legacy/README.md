# Legacy engine and GUI (2020–2021)

This is the original capstone program, kept buildable and playable. The SDL
GUI is genuinely useful — it is still the only way to play Pioneer with a
mouse — but the agent behind it is superseded by `engine/`.

```sh
cd legacy && make run
```

## What the audit found

The MCTS agent here does not search. Three defects compound:

1. Move legality inside `MoveChess` is gated on the `walking` array
   (`interface.cpp`), which is filled by the GUI when a piece is clicked.
   During self-play the render loop clears it every frame and the search
   never rebuilds it, so **every move applied inside the tree is silently
   rejected** and the position never advances past the root.
2. `Node::Expansion` creates children whose move lists come from the parent
   position (`algorithm/mcts.cpp`), so below depth two the edges describe a
   position that is one move stale.
3. Backpropagation mixes an already-signed simulation result with a static
   capture score through `(2 * (result - 0.5) + sigmoid(move.value) * 2) / 3`,
   an asymmetric scale on which the negamax sign flip does not hold. The
   static term dominates, so the tree ranks moves by material grabbed rather
   than by anything it searched.

What looks like playing strength comes from the move filter's preference for
captures, plus a one-ply king capture that works by accident: a rejected move
onto the enemy king still sets `isKingDead_`.

They are not bugs to patch. Fixing them means the position, the search and
the rules all have to be rebuilt, which is what `engine/` is.

## Also worth knowing

- `get_Moves` has its colors reversed in the defence check: `isWillBeKilled`
  detects *our* attack on *their* king, so the agent has no notion of its own
  king being attacked. The last feature commit before the project was
  shelved is titled "improve defense strategy".
- Piece values contradict shogi theory (rook 54 < bishop 60 < silver 64 <
  gold 74, pawn 50 nearly equal to rook).
- Repetition detection exists but is never called, and writes out of bounds.
- The engine class inherits the GUI (`class State : public Interface`), so
  every simulation copies SDL handles along with the board.

The new engine keeps none of this code, only the lesson about which parts of
a game program have to be verifiable: see `docs/verification.md`.
