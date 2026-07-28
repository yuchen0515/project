CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -MMD -MP
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs)

BIN  := Pioneer
SRCS := run.cpp interface.cpp \
        algorithm/state.cpp algorithm/mcts.cpp algorithm/exportHistory.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SDL_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

# The binary resolves its assets relative to the working directory
# (../image), so start it from this directory.
run: $(BIN)
	./$(BIN)

clean:
	$(RM) $(OBJS) $(DEPS) $(BIN)

-include $(DEPS)

.PHONY: all run clean
