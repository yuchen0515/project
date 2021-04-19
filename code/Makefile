all:
	g++ --std=c++17 -c -w ./algorithm/mcts.cpp ./algorithm/state.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 -c -w run.cpp interface.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 run.o interface.o mcts.o state.o -o powerShogi `sdl2-config --cflags --libs`
	./powerShogi
	rm powerShogi *.o
