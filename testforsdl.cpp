#include <iostream>
#include <string>
#include <SDL_image.h>
#include <SDL.h>

using namespace std;

//Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 640;

//declare function
SDL_Surface *loadSurface(string path);
bool init();
void close();
bool loadMedia(SDL_Surface **gHelloWorld, char *str);
void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h);


SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Surface *screenBoard= NULL;
SDL_Surface *screenBackground= NULL;



int main(){

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Get window surface
        screenSurface = SDL_GetWindowSurface(window);
        //Fill the surface green
        SDL_FillRect(screenSurface, NULL, SDL_MapRGBA(screenSurface->format, 255, 255, 255, 255));
        SDL_UpdateWindowSurface(window);

        loadMedia(&screenBackground, (char *)"background.bmp");

        SDL_Rect dest;
        setup_bmp_size(&dest, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        SDL_BlitScaled(screenBackground, NULL, screenSurface, &dest);
        SDL_UpdateWindowSurface(window);



        //Load media
        if( !loadMedia(&screenBoard, (char *)"board.bmp") )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Apply the image
            setup_bmp_size(&dest, 160, 140, 480, 960);

            SDL_BlitScaled(screenBoard, NULL, screenSurface, &dest);
            //Update the surface
            SDL_UpdateWindowSurface(window);
            //Wait two seconds
            SDL_Event e;

            bool quit = false;
            while (!quit){
                while (SDL_PollEvent(&e)){


                    if (e.type == SDL_QUIT)
                        quit = true;

                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        quit = true;

                    //if (e.type == SDL_MOUSEBUTTONDOWN){
                    //    quit = true;
                    //}
                }
            }
        }
    }

    close();

    return 0;
}

void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h){
    dest->x = x;
    dest->y = y;
    dest->w = w;
    dest->h = h;
}

void close()
{
    //Deallocate surface
    SDL_FreeSurface(screenBoard);
    screenBoard = NULL;

    //Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

bool loadMedia(SDL_Surface **gHelloWorld, char *str)
{
    //Loading success flag
    bool success = true;

    //Load splash image
    *gHelloWorld = SDL_LoadBMP(str);

    if (*gHelloWorld == NULL )
    {
        printf("Unable to load image %s!\n", str);
        printf("SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

bool init(){
    //The window we'll be rendering to
    //The surface contained by the window
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL couldn't initialize!");
        printf("SDL_Error detail: %s\n", SDL_GetError());
        return 0;
    }

    //Create window
    window = \
             SDL_CreateWindow( "Mini-shogi",     //window title 
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
    return 1;
}
