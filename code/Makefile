all:
	g++ --std=c++17 -c -w -O3 ./algorithm/exportHistory.cpp
	g++ --std=c++17 -c -w -O3 ./algorithm/state.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 -c -w -O3 ./algorithm/mcts.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 -c -w -O3 interface.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 -c -w -O3 run.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 run.o exportHistory.o interface.o mcts.o state.o -O3 -o Pioneer `sdl2-config --cflags --libs`
	./Pioneer
	rm Pioneer *.o
