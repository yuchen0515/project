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
        InitMedia();


        //Load media
        if( !loadMedia(&gTextureBoard, (char *)"board.bmp") )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Update the surface
            SDL_UpdateWindowSurface(window);
            //Wait two seconds
            SDL_Event e;

            bool quit = false;
            while (!quit){
                //Clear screen
                SDL_RenderClear( gRenderer );

                //cover
                show_walking(MouseIndex);

                while (SDL_PollEvent(&e)){

                    if (e.type == SDL_QUIT)
                        quit = true;

                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        quit = true;

                    if (SDL_MOUSEBUTTONDOWN == e.type){
                        if (SDL_BUTTON_LEFT == e.button.button)
                        {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
                            printf("x, y: %d %d ...............\n", MouseX, MouseY);

                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.second || MouseIndex.second != MouseIndexTemp.second)){
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;
                                //printf("click: %d\n", bClickChess);
                            }else{

                            if (ClickCover(MouseIndex))
                                bClickChess = true;
                            else
                                bClickChess = false;
                            }
                        }
                        else if(SDL_BUTTON_RIGHT == e.button.button)
                        {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
                            printf("x, y: %d %d ...............\n", MouseX, MouseY);
                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.second || MouseIndex.second != MouseIndexTemp.second)){
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;
                                //printf("click: %d\n", bClickChess);
                            }else{

                            if (ClickCover(MouseIndex))
                                bClickChess = true;
                            else
                                bClickChess = false;
                            }
                        }
                        printf("index_x: %d, index_y: %d\n", MouseIndex.first, MouseIndex.second);

                        PrintBugMessageBoard();

                    }


                }

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &NoMove[0]);
                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &NoMove[1]);

                if (bClickChess == false){

                    show_walking(make_pair(-1, -1));
                }else
                    show_walking(MouseIndex);



                Show_Chess();

                ClickCover(MouseIndex);


                //Update screen
                SDL_RenderPresent( gRenderer );

            }
        }
    }


    close();

    return 0;
}
