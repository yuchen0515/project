CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -g -Wall -Wextra -MMD -MP

LIB_SRCS  := $(wildcard core/*.cpp) $(wildcard agents/*.cpp) \
             $(wildcard arena/*.cpp) usi/usi.cpp
TEST_SRCS := $(wildcard tests/*.cpp)
MAIN_SRCS := usi/usi_main.cpp tools/perft_main.cpp tools/arena_main.cpp \
             tools/play_main.cpp

LIB_OBJS  := $(LIB_SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)
MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)
DEPS      := $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(MAIN_OBJS:.o=.d)

TEST_BIN  := tests/run_tests
USI_BIN   := pioneer
PERFT_BIN := tools/perft
ARENA_BIN := tools/arena
PLAY_BIN  := tools/play
GUI_BIN   := tools/gui
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null)

# Binaries first, then tests. With tests as a prerequisite of the
# binaries, a failing test aborts the build and leaves stale tools in
# place - which silently invalidates whatever you measure with them.
all: build test

build: $(TEST_BIN) $(USI_BIN) $(PERFT_BIN) $(ARENA_BIN) $(PLAY_BIN)

$(TEST_BIN): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(USI_BIN): $(LIB_OBJS) usi/usi_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(PERFT_BIN): $(LIB_OBJS) tools/perft_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ARENA_BIN): $(LIB_OBJS) tools/arena_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(PLAY_BIN): $(LIB_OBJS) tools/play_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Not part of `all`: the engine itself has no dependencies and should
# still build on a machine without SDL2.
gui: $(GUI_BIN)

$(GUI_BIN): $(LIB_OBJS) gui/gui_main.cpp
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $^ -o $@ $(SDL_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	$(RM) $(LIB_OBJS) $(TEST_OBJS) $(MAIN_OBJS) $(DEPS) \
	      $(TEST_BIN) $(USI_BIN) $(PERFT_BIN) $(ARENA_BIN) $(PLAY_BIN) $(GUI_BIN)

-include $(DEPS)

.PHONY: all build test clean gui
