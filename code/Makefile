all:
	g++ --std=c++17 -c -w run.cpp interface.cpp `sdl2-config --cflags --libs`
	g++ --std=c++17 run.o interface.o -o run `sdl2-config --cflags --libs`
	./run
	rm run *.o
