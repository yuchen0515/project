/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學
 * 資訊工程學系
 */

#include <iostream>
#include "interface.h"

#ifndef PRINT
#define PRINT
#endif

// Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 640;

SDL_Surface *screenBoard = nullptr;
SDL_Surface *screenSurface = nullptr;
SDL_Window *window = nullptr;
SDL_Rect temp;

// current dusokated textyre
SDL_Texture *gTextureBoard = nullptr;
SDL_Texture *gTextureBackground = nullptr;
SDL_Texture *gTextureKing = nullptr;
SDL_Texture *gTextureJade = nullptr;
SDL_Texture *gTextureGold = nullptr;
SDL_Texture *gTextureSliver = nullptr;
SDL_Texture *gTextureSliverUp = nullptr;
SDL_Texture *gTextureBishop = nullptr;
SDL_Texture *gTextureBishopUp = nullptr;
SDL_Texture *gTexturePawn = nullptr;
SDL_Texture *gTexturePawnUp = nullptr;
SDL_Texture *gTextureRook = nullptr;
SDL_Texture *gTextureRookUp = nullptr;
SDL_Texture *gTextureAlphaChess = nullptr;
SDL_Texture *gTextureLatticeCover = nullptr;
SDL_Texture *gTextureShow = nullptr;

// 0: upper, 1: lower
//
// 1: king, 2: rook, 3: bishop, 4: gold
// 5: sliver, 6: pawn
// 7: rook (up), 8: bishop (up), 9: reserve
// 10: sliver (up), 11: pawn(up)
// 12 ~ 17: captive 1~6
//
int32_t exist[2][5][7];
int32_t walking[5][5];

// The window renderer
SDL_Renderer *gRenderer = NULL;

pair<int32_t, int32_t> MouseIndex = make_pair(-1, -1);
pair<int32_t, int32_t> MouseIndexTemp = make_pair(-1, -1);
int32_t MouseX = 0, MouseY = 0;
bool bClickChess = false;
bool KingDead = false;

SDL_Rect NoMove[2];
SDL_Rect ChessDect[5][9];
SDL_Point ChessSize = (SDL_Point){25, 30};

int main() {
    // Start up SDL and create window
    if (init() == false) {
        std::cout << "Failed to initialize!" << std::endl;
    } else {
        InitMedia();

        // Load media
        if (loadMedia(&gTextureBoard,
                    const_cast<char *> ("board.bmp")) == false) {
            std::cout << "Failed to load media!" << std::endl;
        } else {
            // Update the surface
            SDL_UpdateWindowSurface(window);

            // Wait two seconds
            SDL_Event e;

            bool quit = false;

            while (quit == false) {
                // Clear screen
                SDL_RenderClear(gRenderer);

                // cover
                show_walking(MouseIndex);

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    }

                    if (e.key.keysym.sym == SDLK_a) {
                        InitExist();
                        KingDead = false;
                    }

                    if (KingDead == true) {
                        usleep(50000);
                        continue;
                    }

                    if (SDL_MOUSEBUTTONDOWN == e.type) {
                        if (SDL_BUTTON_LEFT == e.button.button) {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
#ifdef PRINT
                            std::cout << "x, y: " << MouseX;
                            std::cout << " " << MouseY;
                            std::cout << "..............." << std::endl;
#endif

                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.first || MouseIndex.second != MouseIndexTemp.second)) {
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;

                                if (DetectKingExist() == 0) {
                                    KingDead = true;
                                }
                            } else if (ClickCover(MouseIndex) && bClickChess == true && (MouseIndex.first == MouseIndexTemp.first && MouseIndex.second == MouseIndexTemp.second)) {
                                bClickChess = false;
                            } else {
                                bClickChess = ClickCover(MouseIndex) ? 1 : 0;
                            }
                        } else if (SDL_BUTTON_RIGHT == e.button.button) {
                            MouseIndexTemp = MouseIndex;
                            MouseX = e.button.x;
                            MouseY = e.button.y;
                            MouseIndex = return_MouseIndex(MouseX, MouseY);
#ifdef PRINT
                            std::cout << "x, y: " << MouseX;
                            std::cout << " " << MouseY;
                            std::cout << "..............." << std::endl;
#endif
                            if (bClickChess && (MouseIndex.first != MouseIndexTemp.first|| MouseIndex.second != MouseIndexTemp.second)) {
                                MoveChess(MouseIndexTemp, MouseIndex);
                                bClickChess = false;

                                if (DetectKingExist() == 0)
                                    KingDead = true;
                            }
                            else if (ClickCover(MouseIndex) && bClickChess == true && (MouseIndex.first == MouseIndexTemp.first && MouseIndex.second == MouseIndexTemp.second)){
                                bClickChess = false;
                            }
                            else{
                                bClickChess = ClickCover(MouseIndex) ? 1 : 0;
                            }
                        }
                        std::cout << "index_x: " << MouseIndex.first << ", ";
                        std::cout << MouseIndex.second << std::endl;

                        PrintBugMessageBoard();

                    }

                }

                //Render texture to screen
                SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &NoMove[0]);
                //Apply the image
                SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &NoMove[1]);

                if (bClickChess == false) {
                    show_walking(make_pair(-1, -1));
                } else {
                    show_walking(MouseIndex);
                }

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
