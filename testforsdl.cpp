#include <iostream>
#include <string>
#include <SDL_image.h>
#include <SDL.h>

using namespace std;

//Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 640;

//declare function
SDL_Surface *loadSurface(char *str);
SDL_Surface *screenBoard= NULL;

bool init();
void close();
bool loadMedia(SDL_Surface **gHelloWorld, char *str);
bool loadMedia(SDL_Texture **gTexture, char *str);
void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h);
SDL_Texture *loadTexture(char *str);


SDL_Window *window = NULL;
SDL_Surface *screenSurface = NULL;

//The window renderer
SDL_Renderer *gRenderer = NULL;

//current dusokated textyre
SDL_Texture *gTextureBoard = NULL;
SDL_Texture *gTextureBackground= NULL;
SDL_Texture *gTextureKing= NULL;
SDL_Texture *gTextureJade= NULL;
SDL_Texture *gTextureGold= NULL;
SDL_Texture *gTextureSliver= NULL;
SDL_Texture *gTextureBishop= NULL;
SDL_Texture *gTexturePawn= NULL;
SDL_Texture *gTextureRook= NULL;

SDL_Rect No_Move[2];
SDL_Rect Chess_Dect[10];
SDL_Point Chess_Size = (SDL_Point){25, 30};

int main(){

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Get window surface
        
        //screenSurface = SDL_GetWindowSurface(window);
        ////Fill the surface green
        //SDL_FillRect(screenSurface, NULL, SDL_MapRGBA(screenSurface->format, 255, 255, 255, 255));
        //SDL_UpdateWindowSurface(window);

        //loadMedia(&screenBackground, (char *)"background.bmp");


        loadMedia(&gTextureBackground, (char *)"background.bmp");
        loadMedia(&gTextureKing   , (char *)"king.bmp");
        loadMedia(&gTextureJade   , (char *)"Jade.bmp"); 
        loadMedia(&gTextureGold   , (char *)"gold.bmp"); 
        loadMedia(&gTextureSliver , (char *)"sliver.bmp");   
        loadMedia(&gTextureBishop , (char *)"bishop.bmp");   
        loadMedia(&gTexturePawn   , (char *)"pawn.bmp"); 
        loadMedia(&gTextureRook   , (char *)"rook.bmp"); 
        //SDL_BlitScaled(screenBackground, NULL, screenSurface, &dest);
        //SDL_UpdateWindowSurface(window);

        //Load media
        if( !loadMedia(&gTextureBoard, (char *)"board.bmp") )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {

            //SDL_BlitScaled(screenBoard, NULL, screenSurface, &dest);
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

                }
                //Clear screen
                SDL_RenderClear( gRenderer );

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &No_Move[0]);

                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &No_Move[1]);


                SDL_RenderCopyEx(gRenderer, gTextureRook, NULL, &Chess_Dect[0], 180, &Chess_Size, SDL_FLIP_NONE);
                SDL_RenderCopyEx(gRenderer, gTextureBishop, NULL, &Chess_Dect[1], 180, &Chess_Size, SDL_FLIP_NONE);

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }


    close();

    return 0;
}

bool init(){

    setup_bmp_size(&No_Move[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    setup_bmp_size(&No_Move[1], 160, 140, 480, 960);

    //Rook
    setup_bmp_size(&Chess_Dect[0], 0, 0, 125, 250);

    //bishop
    setup_bmp_size(&Chess_Dect[1], 140, 160, 130, 260);
    //

    //The window we'll be rendering to
    //The surface contained by the window
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL couldn't initialize!");
        printf("SDL_Error detail: %s\n", SDL_GetError());
        return 0;
    }

    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
        printf( "Warning: Linear texture filtering not enabled!" );
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

    //Create renderer for window
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if( gRenderer == NULL )
    {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }
    //Initialize renderer color
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

    ////Initialize PNG loading
    //int imgFlags = IMG_INIT_PNG;
    //if( !( IMG_Init( imgFlags ) & imgFlags ) )
    //{
    //    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
    //    success = false;
    //}

    return 1;
}

void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h){
    dest->x = x;
    dest->y = y;
    dest->w = w;
    dest->h = h;
}

bool loadMedia(SDL_Surface **gHelloWorld, char *str){
    //Loading success flag
    bool success = true;

    //Load splash image
    *gHelloWorld = loadSurface(str);

    if (*gHelloWorld == NULL )
    {
        printf("Unable to load image %s!\n", str);
        printf("SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

bool loadMedia(SDL_Texture **gTexture, char *str){
    //Loading success flag
    bool success = true;

    //Load splash image
    *gTexture= loadTexture(str);

    if (*gTexture == NULL )
    {
        printf("Unable to load image %s!\n", str);
        printf("SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

SDL_Surface* loadSurface( char *str ){
    //The final optimized image
    SDL_Surface* optimizedSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", str, SDL_GetError() );
    }
    else
    {
        //Convert surface to screen format
        optimizedSurface = SDL_ConvertSurface(loadedSurface, screenSurface->format, 0 );
        if( optimizedSurface == NULL )
        {
            printf( "Unable to optimize image %s! SDL Error: %s\n", str, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return optimizedSurface;
}

SDL_Texture *loadTexture(char *str){
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);

    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", str, SDL_GetError());
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", str, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

void close(){
    //Free loaded image
    SDL_DestroyTexture( gTextureBoard );
    gTextureBoard = NULL;

    //Deallocate surface
    SDL_FreeSurface(screenBoard);
    screenBoard = NULL;

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow(window);
    window = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}
