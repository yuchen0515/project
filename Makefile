all:
	gcc testforsdl.c -o testforsdl `sdl2-config --cflags --libs`
