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
                            mouse_X = e.button.x;
                            mouse_Y = e.button.y;
                            printf("x, y %d %d ...............\n", mouse_X, mouse_Y);

                        }
                        else if(SDL_BUTTON_RIGHT == e.button.button)
                        {
                            mouse_X = e.button.x;
                            mouse_Y = e.button.y;
                            printf("x, y %d %d ...............\n", mouse_X, mouse_Y);
                        }
                        printf("index_x: %d, index_y: %d\n", mouse_index.first, mouse_index.second);
                    }
                    else if (SDL_MOUSEMOTION == e.type)
                    {
                        //mouse_X = e.button.x;
                        //mouse_Y = e.button.y;
                        //printf("x, y %d %d ...............\n", mouse_X, mouse_Y);
                    }
                    mouse_index = return_mouse_index(mouse_X, mouse_Y);
                }
                //Clear screen
                SDL_RenderClear( gRenderer );

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &No_Move[0]);

                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &No_Move[1]);



                //cover
                show_walking(mouse_index);


                Show_Chess();

                if (mouse_index.first >= 0 && mouse_index.first <= 4 && mouse_index.second >= 0 && mouse_index.second <= 4){
                    if (exist[1][mouse_index.first][mouse_index.second] > 0)
                        SDL_RenderCopy(gRenderer, gTextureAlphaChess, NULL, &Chess_Dect[mouse_index.first][mouse_index.second]); 
                    else if (exist[0][mouse_index.first][mouse_index.second] > 0)
                        SDL_RenderCopyEx(gRenderer, gTextureAlphaChess, NULL, &Chess_Dect[mouse_index.first][mouse_index.second], 180, &Chess_Size, SDL_FLIP_NONE); 
                }

                //Update screen
                SDL_RenderPresent( gRenderer );
            }
        }
    }


    close();

    return 0;
}

