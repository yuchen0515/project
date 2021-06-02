#pragma once

//#ifndef MCTS_H_INCLUDED
//#define MCTS_H_INCLUDED

///////////////////////

#include "state.h"

#ifndef MOVE
#define MOVE

typedef struct move {
    std::pair<int32_t, int32_t> from;
    std::pair<int32_t, int32_t> to;
}Move;

#endif
///////////////////////

class State;

namespace PURE {
    Move MCTS(State state, int32_t simLimit);
}

//#endif
