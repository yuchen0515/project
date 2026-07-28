CXX      ?= g++
CXXFLAGS := -std=c++17 -O2 -g -Wall -Wextra -MMD -MP

CORE_SRCS := $(wildcard core/*.cpp)
TEST_SRCS := $(wildcard tests/*.cpp)
CORE_OBJS := $(CORE_SRCS:.cpp=.o)
TEST_OBJS := $(TEST_SRCS:.cpp=.o)
DEPS      := $(CORE_OBJS:.o=.d) $(TEST_OBJS:.o=.d) tools/perft_main.d

TEST_BIN  := tests/run_tests
PERFT_BIN := tools/perft

all: test $(PERFT_BIN)

$(TEST_BIN): $(CORE_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(PERFT_BIN): $(CORE_OBJS) tools/perft_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	$(RM) $(CORE_OBJS) $(TEST_OBJS) tools/perft_main.o $(DEPS) \
	      $(TEST_BIN) $(PERFT_BIN)

-include $(DEPS)

.PHONY: all test clean
