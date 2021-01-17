#include "interface.h"

using namespace std;

//Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 640;

SDL_Surface *screenBoard= NULL;
SDL_Surface *screenSurface = NULL;
SDL_Window *window = NULL;
SDL_Rect temp;

//current dusokated textyre
SDL_Texture *gTextureBoard = NULL;
SDL_Texture *gTextureBackground= NULL;
SDL_Texture *gTextureKing= NULL;
SDL_Texture *gTextureJade= NULL;
SDL_Texture *gTextureGold= NULL;
SDL_Texture *gTextureSliver= NULL;
SDL_Texture *gTextureSliverUp= NULL;
SDL_Texture *gTextureBishop= NULL;
SDL_Texture *gTextureBishopUp= NULL;
SDL_Texture *gTexturePawn= NULL;
SDL_Texture *gTexturePawnUp= NULL;
SDL_Texture *gTextureRook= NULL;
SDL_Texture *gTextureRookUp= NULL;
SDL_Texture *gTextureAlphaChess= NULL;
SDL_Texture *gTextureLatticeCover= NULL;
SDL_Texture *gTextureShow= NULL;

//0: upper, 1: lower
//
//1: king, 2: rook, 3: bishop, 4: gold
//5: sliver, 6: pawn
//7: rook (up), 8: bishop (up), 9: reserve
//10: sliver (up), 11: pawn(up)
//12 ~ 17: captive 1~6
//
int32_t exist[2][5][7];
int32_t walking[5][5];

//The window renderer
SDL_Renderer *gRenderer = NULL;

pair<int32_t, int32_t> MouseIndex = make_pair(-1, -1);
pair<int32_t, int32_t> MouseIndexTemp = make_pair(-1, -1);
int32_t MouseX = 0, MouseY = 0;
bool bClickChess = false;
bool KingDead = false;

SDL_Rect NoMove[2];
SDL_Rect ChessDect[5][9];
SDL_Point ChessSize = (SDL_Point){25, 30};

int main(){

    //Start up SDL and create window
    if(!init())
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

                    if (e.key.keysym.sym == SDLK_a){
                        InitExist();
                        KingDead = false;
                    }

                    if (KingDead == true){

                        usleep(50000);
                        continue;
                    }

                    if (SDL_MOUSEBUTTONDOWN == e.type){
                        if (SDL_BUTTON_LEFT == e.button.button)
                        {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
                            printf("x, y: %d %d ...............\n", MouseX, MouseY);

                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.first|| MouseIndex.second != MouseIndexTemp.second)){
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;

                                if (DetectKingExist() == 0)
                                    KingDead = true;
                            }
                            else if (ClickCover(MouseIndex) && bClickChess == true && (MouseIndex.first == MouseIndexTemp.first && MouseIndex.second == MouseIndexTemp.second)){
                                bClickChess = false;
                            }
                            else
                                bClickChess = ClickCover(MouseIndex) ? 1 : 0;
                        }
                        else if(SDL_BUTTON_RIGHT == e.button.button)
                        {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
                            printf("x, y: %d %d ...............\n", MouseX, MouseY);
                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.first|| MouseIndex.second != MouseIndexTemp.second)){
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;

                                if (DetectKingExist() == 0)
                                    KingDead = true;
                            }
                            else if (ClickCover(MouseIndex) && bClickChess == true && (MouseIndex.first == MouseIndexTemp.first && MouseIndex.second == MouseIndexTemp.second)){
                                bClickChess = false;
                            }
                            else
                                bClickChess = ClickCover(MouseIndex) ? 1 : 0;
                        }
                        printf("index_x: %d, index_y: %d\n", MouseIndex.first, MouseIndex.second);

                        PrintBugMessageBoard();

                    }


                }

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &NoMove[0]);
                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &NoMove[1]);

                if (bClickChess == false)
                    show_walking(make_pair(-1, -1));
                else
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
