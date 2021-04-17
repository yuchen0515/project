#ifndef MCTS_H_INCLUDED
#define MCTS_H_INCLUDED

#include "game.h"

namespace PURE {
	Move MCTS(State state, int32_t simLimit);
}

namespace MCTSS {
	Move MCTS(State state, int32_t simLimit);
}

#endif
