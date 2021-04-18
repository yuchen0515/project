/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學 National Taiwan Normal University
 * 資訊工程學系 Computer Science and Information Engineering
 */

#pragma once

#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <utility>
#include "./algorithm/state.h"

class Interface {
    public:
        // Declare function
        SDL_Surface *loadSurface(char const *str);

        bool init();
        void close();
        void InitMedia();
        void InitPosition();
        void InitExist();

        bool loadMedia(
                SDL_Surface **gHelloWorld,
                char const *str);

        bool loadMedia(
                SDL_Texture **gTexture,
                char const *str);

        SDL_Texture *loadTexture(char const *str);

        void Show_Chess();

        void show_walking(
                const std::pair<int32_t, int32_t> temp);

        void Determine_Draw(
                const int32_t kind,
                const int32_t Isupper,
                const int32_t j,
                const int32_t k);

        SDL_Rect* return_lattice_rect(
                const int32_t x,
                const int32_t y);

        bool ClickCover(
                const std::pair<int32_t, int32_t> fMouseIndex) const;

        void MoveChess(
                std::pair<int32_t, int32_t> ori,
                std::pair<int32_t, int32_t> des);

        void CaptivePush(
                const int32_t kind,
                int32_t chess);

        void PrintBugMessageBoard() const;
        bool DetectKingExist();


        std::pair<int32_t, int32_t> return_MouseIndex(
                const int32_t x,
                const int32_t y) const;

        void setup_bmp_size(
                SDL_Rect *dest,
                const int32_t x,
                const int32_t y,
                const int32_t w,
                const int32_t h) const;

        bool match_rect_xy(
                const int32_t& x,
                const int32_t& y,
                const SDL_Rect& rect) const;

        int32_t check_bound_xy(
                const int32_t cur_x,
                const int32_t cur_y,
                const int32_t add_x,
                const int32_t add_y,
                const int32_t upper) const;

        void run();

    private:
        // Screen dimension constants  螢幕寬高設定
        const int32_t SCREEN_WIDTH = 640;
        const int32_t SCREEN_HEIGHT = 640;
        static constexpr int32_t M_ = 5;
        static constexpr int32_t N_ = 5;
        static constexpr int32_t PLAYER_NUMBER_ = 2;

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
        int32_t exist[PLAYER_NUMBER_][N_][7] = {0};
        std::vector<std::vector<int32_t>> walking{M_, std::vector<int32_t>(N_, 0)};

        // The window renderer
        SDL_Renderer *gRenderer = nullptr;

        std::pair<int32_t, int32_t> mouseIndex_ = std::make_pair(-1, -1);
        std::pair<int32_t, int32_t> mouseIndexTemp_ = std::make_pair(-1, -1);
        int32_t mouseX_ = 0;
        int32_t mouseY_ = 0;
        bool isClickChess_ = false;
        bool isKingDead_ = false;

        SDL_Rect texPosition_[2];
        SDL_Rect chessDect_[6][9];
        SDL_Point chessSize_ = (SDL_Point){25, 30};

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
            PAWNUP_ = 11 };
};
