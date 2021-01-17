#pragma once
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

using namespace std;

//Screen dimension constants  螢幕寬高設定
extern const int32_t SCREEN_WIDTH;
extern const int32_t SCREEN_HEIGHT;

extern SDL_Surface *screenBoard;
extern SDL_Surface *screenSurface;
extern SDL_Window *window;
extern SDL_Rect temp;

//current dusokated textyre
extern SDL_Texture *gTextureBoard ;
extern SDL_Texture *gTextureBackground;
extern SDL_Texture *gTextureKing;
extern SDL_Texture *gTextureJade;
extern SDL_Texture *gTextureGold;
extern SDL_Texture *gTextureSliver;
extern SDL_Texture *gTextureSliverUp;
extern SDL_Texture *gTextureBishop;
extern SDL_Texture *gTextureBishopUp;
extern SDL_Texture *gTexturePawn;
extern SDL_Texture *gTexturePawnUp;
extern SDL_Texture *gTextureRook;
extern SDL_Texture *gTextureRookUp;
extern SDL_Texture *gTextureAlphaChess;
extern SDL_Texture *gTextureLatticeCover;
extern SDL_Texture *gTextureShow;

//0: upper, 1: lower
//
//1: king, 2: rook, 3: bishop, 4: gold
//5: sliver, 6: pawn
//7: rook (up), 8: bishop (up), 9: reserve
//10: sliver (up), 11: pawn(up)
//12 ~ 17: captive 1~6
//
extern int32_t exist[2][5][7];


extern int32_t walking[5][5];

//The window renderer
extern SDL_Renderer *gRenderer;

extern pair<int32_t, int32_t> MouseIndex;
extern pair<int32_t, int32_t> MouseIndexTemp;
extern int32_t MouseX, MouseY;
extern bool bClickChess;
extern bool KingDead;


extern SDL_Rect NoMove[2];
extern SDL_Rect ChessDect[5][9];
extern SDL_Point ChessSize;

//declare function
SDL_Surface *loadSurface(char *str);

bool init();
void close();
void InitMedia();
void InitPosition();
void InitExist();

bool loadMedia(SDL_Surface **gHelloWorld, char *str);
bool loadMedia(SDL_Texture **gTexture, char *str);
SDL_Texture *loadTexture(char *str);

void Show_Chess();
void show_walking(pair<int32_t, int32_t> temp);
void Determine_Draw(int32_t kind, int32_t Isupper);
SDL_Rect *return_lattice_rect(int32_t x, int32_t y);
bool ClickCover(pair<int32_t, int32_t> fMouseIndex);
void MoveChess(pair<int32_t, int32_t> ori, pair<int32_t, int32_t> des);
void CaptivePush(int32_t kind, int32_t chess);
void PrintBugMessageBoard();
bool DetectKingExist();


pair<int32_t, int32_t> return_MouseIndex(int32_t x, int32_t y);
void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h);
bool match_rect_xy(int32_t x, int32_t y, SDL_Rect rect);
int32_t check_bound_xy(int32_t cur_x, int32_t cur_y, int32_t add_x, int32_t add_y, int32_t upper);
