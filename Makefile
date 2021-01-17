all:
	gcc run.cpp interface.cpp -o run `sdl2-config --cflags --libs`
	./run
