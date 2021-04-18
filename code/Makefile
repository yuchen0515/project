all:
	g++ --std=c++17 -c -w run.cpp interface.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 run.o interface.o -o powerShogi `sdl2-config --cflags --libs`
	./powerShogi
	rm powerShogi *.o
