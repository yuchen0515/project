all:
	g++ testforsdl.cpp -o testforsdl `sdl2-config --cflags --libs`
	./testforsdl
