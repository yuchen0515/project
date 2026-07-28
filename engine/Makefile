CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -g -Wall -Wextra -MMD -MP

LIB_SRCS  := $(wildcard core/*.cpp) $(wildcard agents/*.cpp) usi/usi.cpp
TEST_SRCS := $(wildcard tests/*.cpp)
MAIN_SRCS := usi/usi_main.cpp tools/perft_main.cpp

LIB_OBJS  := $(LIB_SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)
MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)
DEPS      := $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(MAIN_OBJS:.o=.d)

TEST_BIN  := tests/run_tests
USI_BIN   := pioneer
PERFT_BIN := tools/perft

all: test $(USI_BIN) $(PERFT_BIN)

$(TEST_BIN): $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(USI_BIN): $(LIB_OBJS) usi/usi_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(PERFT_BIN): $(LIB_OBJS) tools/perft_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	$(RM) $(LIB_OBJS) $(TEST_OBJS) $(MAIN_OBJS) $(DEPS) \
	      $(TEST_BIN) $(USI_BIN) $(PERFT_BIN)

-include $(DEPS)

.PHONY: all test clean
