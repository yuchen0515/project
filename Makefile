all:
	gcc main.cpp -o main `sdl2-config --cflags --libs`
	./main
