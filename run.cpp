#include "interface.h"

using namespace std;

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

        InitMedia();

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

                    if (SDL_MOUSEBUTTONDOWN == e.type){
                        if (SDL_BUTTON_LEFT == e.button.button)
                        {
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            printf("x, y %d %d ...............\n", MouseX, MouseY);

                        }
                        else if(SDL_BUTTON_RIGHT == e.button.button)
                        {
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            printf("x, y %d %d ...............\n", MouseX, MouseY);
                        }
                        printf("index_x: %d, index_y: %d\n", MouseIndex.first, MouseIndex.second);
                    }
                    else if (SDL_MOUSEMOTION == e.type)
                    {
                        //MouseX = e.button.x;
                        //MouseY = e.button.y;
                        //printf("x, y %d %d ...............\n", MouseX, MouseY);
                    }
                    MouseIndex = return_MouseIndex(MouseX, MouseY);
                }
                //Clear screen
                SDL_RenderClear( gRenderer );

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &NoMove[0]);

                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &NoMove[1]);



                //cover
                show_walking(MouseIndex);


                Show_Chess();

                if (MouseIndex.first >= 0 && MouseIndex.first <= 4 && MouseIndex.second >= 0 && MouseIndex.second <= 4){
                    if (exist[1][MouseIndex.first][MouseIndex.second] > 0)
                        SDL_RenderCopy(gRenderer, gTextureAlphaChess, NULL, &ChessDect[MouseIndex.first][MouseIndex.second]); 
                    else if (exist[0][MouseIndex.first][MouseIndex.second] > 0)
                        SDL_RenderCopyEx(gRenderer, gTextureAlphaChess, NULL, &ChessDect[MouseIndex.first][MouseIndex.second], 180, &ChessSize, SDL_FLIP_NONE); 
                }

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }


    close();

    return 0;
}

