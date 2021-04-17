/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學
 * 資訊工程學系
 */

#pragma once

#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <utility>

class Interface {
    public:
        // Declare function
        SDL_Surface *loadSurface(char *str);

        bool init();
        void close();
        void InitMedia();
        void InitPosition();
        void InitExist();

        bool loadMedia(
                SDL_Surface **gHelloWorld,
                char *str);

        bool loadMedia(
                SDL_Texture **gTexture,
                char *str);

        SDL_Texture *loadTexture(char *str);

        void Show_Chess();

        void show_walking(
                std::pair<int32_t, int32_t> temp);

        void Determine_Draw(
                int32_t kind,
                int32_t Isupper,
                int32_t j,
                int32_t k);

        SDL_Rect *return_lattice_rect(
                int32_t x,
                int32_t y);

        bool ClickCover(
                std::pair<int32_t, int32_t> fMouseIndex);

        void MoveChess(
                std::pair<int32_t, int32_t> ori,
                std::pair<int32_t, int32_t> des);

        void CaptivePush(
                int32_t kind,
                int32_t chess);

        void PrintBugMessageBoard();
        bool DetectKingExist();


        std::pair<int32_t, int32_t> return_MouseIndex(
                int32_t x,
                int32_t y);

        void setup_bmp_size(
                SDL_Rect *dest,
                int32_t x,
                int32_t y,
                int32_t w,
                int32_t h);

        bool match_rect_xy(
                int32_t x,
                int32_t y,
                SDL_Rect rect);

        int32_t check_bound_xy(
                int32_t cur_x,
                int32_t cur_y,
                int32_t add_x,
                int32_t add_y,
                int32_t upper);

        void run();

    private:
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
        int32_t exist[2][5][7] = {0};
        int32_t walking[5][5] = {0};

        // The window renderer
        SDL_Renderer *gRenderer = nullptr;

        std::pair<int32_t, int32_t> MouseIndex = std::make_pair(-1, -1);
        std::pair<int32_t, int32_t> MouseIndexTemp = std::make_pair(-1, -1);
        int32_t MouseX = 0, MouseY = 0;
        bool bClickChess = false;
        bool KingDead = false;

        SDL_Rect NoMove[2];
        SDL_Rect ChessDect[6][9];
        SDL_Point ChessSize = (SDL_Point){25, 30};

        enum chessType_ {
            KING_ = 1,
            ROOK_ = 2,
            BISHOP_ = 3,
            GOLD_ = 4,
            SLIVER_ = 5,
            PAWN_ = 6,
            ROOKUP_ = 7,
            BISHOPUP_ = 8,
            SLIVERUP_ = 9,
            PAWNUP_ = 10 };
};
