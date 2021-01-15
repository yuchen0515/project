#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

//Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 480;

int main(){

	//The window we'll be rendering to
	SDL_Window *window = NULL;

	//The surface contained by the window
	SDL_Surface *screenSurface = NULL;

	//Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL couldn't initialize!");
        printf("SDL_Error detail: %s\n", SDL_GetError());
		return 0;
    }
	
	//Create window
    window = \
             SDL_CreateWindow( "Hello SDL",     //window title 
                     SDL_WINDOWPOS_CENTERED,    //position setting X
                     SDL_WINDOWPOS_CENTERED,    //position setting Y
                     SCREEN_WIDTH,              //window width
                     SCREEN_HEIGHT,             //window height
                     SDL_WINDOW_SHOWN           //show window
                     );

    if (window == NULL){
        printf("Window couldn't be created!\n");
        printf("SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    //Get window surface
    screenSurface = SDL_GetWindowSurface(window);

    //Fill the surface green
    SDL_FillRect(screenSurface, NULL, SDL_MapRGBA(screenSurface->format, 0, 255, 255, 0));

    //Update the surface
    SDL_UpdateWindowSurface(window);



    SDL_Event e;

    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                quit = true;
            }
        }
    }

    //You shouldn't wait...... the window after disappearing
    //SDL_Delay( 2000 );

    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}
