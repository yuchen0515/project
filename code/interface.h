/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學 National Taiwan Normal University
 * 資訊工程學系 Computer Science and Information Engineering
 */

#pragma once

#include <random>
#include <chrono>
#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <utility>

//#include "./algorithm/mcts.h"

#ifndef MOVE
#define MOVE

typedef struct move {
    std::pair<int32_t, int32_t> from;
    std::pair<int32_t, int32_t> to;
    int32_t value;
}Move;

#endif

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

        int32_t get_turns() const {
            return turn_;
        }

        void Show_Chess();

        bool is_End() const {
            return isKingDead_ == true;
        }

        std::vector<Move> get_Moves()  const {
            std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
            //move_.clear();
            std::vector<std::vector<int32_t>> walking(COL_SIZE_, std::vector<int32_t>(ROW_SIZE_, 0));

            std::vector<Move> move__;
           
            Move TEMP;
            for (int32_t i = 0 ; i < 5 ; i++){
                for (int32_t j = 0 ; j < 5 ; j ++) {
                    TEMP.from = std::make_pair(i, j);
                    make_walking(TEMP.from, walking);
                    for (int32_t k = 0 ; k < ROW_SIZE_ ; k ++){
                        for (int32_t p = 0 ; p < COL_SIZE_ ; p ++){
                            if (walking[k][p] == 1){
                                TEMP.to = std::make_pair(k, p);
                                //TEMP.value = evl_value(get_turns() == 1 ? 0 : 1, TEMP.to) - evl_value(get_turns(), TEMP.from) * 0.9;
                                TEMP.value = evl_value(get_turns() == 1 ? 0 : 1, TEMP.to);
                                TEMP.value -= evl_value(get_turns(), TEMP.from);
                                move__.emplace_back(TEMP);
                            }
                        }
                    }
                }
            }
            struct cmp{
                bool operator()(const Move& a, const Move& b){
                    if (a.value > b.value){
                        return a.value > b.value;
                    } 
                    //else if (a.value == b.value){
                    //    return rand() % 2 == 0;
                    //}
                    //return false;
                }
            };

            shuffle(move__.begin(), move__.end(), rng);
            //sort(move__.begin(), move__.end(), cmp());
            return move__;
        }
        int32_t evl_value(int32_t turn, std::pair<int32_t, int32_t> temp)const {
            auto comp = exist[turn][temp.first][temp.second];

            switch (comp) {
                case KING_:
                    return 300000;
                case ROOK_:
                    return 54;
                case BISHOP_:
                    return 60;
                case GOLD_:
                    return 74;
                case SLIVER_:
                    return 64;
                case PAWN_:
                    return 50;
                case ROOKUP_:
                    return 80;
                case BISHOPUP_:
                    return 84;
                case SLIVERUP_:
                case PAWNUP_:
                    return 65;
            }
            return 0;
        }

        void make_walking(
                const std::pair<int32_t, int32_t> temp,
                std::vector<std::vector<int32_t>>& walking) const;

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
        bool DetectKingExist() const ;


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

        //void run();
        void Agent();

    protected:
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

        // The window renderer
        SDL_Renderer *gRenderer = nullptr;

        int32_t mouseX_ = 0;
        int32_t mouseY_ = 0;

        bool isWithinBound (std::pair<int32_t, int32_t> TEMP) const;

        bool isClickChess_ = false;
        SDL_Rect texPosition_[2];
        SDL_Rect chessDect_[6][9];
        SDL_Point chessSize_ = (SDL_Point){25, 30};

        static constexpr int32_t PLAYER_NUMBER_ = 2;
        static constexpr int32_t LEVEL_CHANGE_ = 5;
        static constexpr int32_t COL_SIZE_ = 5;
        static constexpr int32_t ROW_SIZE_ = 5;

        // 0: upper, 1: lower
        //
        // 1: king, 2: rook, 3: bishop, 4: gold
        // 5: sliver, 6: pawn
        // 7: rook (up), 8: bishop (up), 9: reserve
        // 10: sliver (up), 11: pawn(up)
        // 12 ~ 17: captive 1~6
        //
        //int32_t exist[PLAYER_NUMBER_][ROW_SIZE_][7] = {0};
        std::vector<std::vector<std::vector<int32_t>>> exist{PLAYER_NUMBER_, std::vector<std::vector<int32_t>>(COL_SIZE_, std::vector<int32_t>(7, 0))};
        std::vector<std::vector<int32_t>> walking{COL_SIZE_, std::vector<int32_t>(ROW_SIZE_, 0)};

        std::pair<int32_t, int32_t> mouseIndex_ = std::make_pair(-1, -1);
        std::pair<int32_t, int32_t> mouseIndexTemp_ = std::make_pair(-1, -1);
        int32_t turn_ = 1;

        bool isKingDead_ = false;

        std::vector<Move> move_;

        enum chessType_ {
            KING_ = 1,
            ROOK_,
            BISHOP_ = 3,
            GOLD_,
            SLIVER_ = 5,
            PAWN_,
            ROOKUP_ = 7,
            BISHOPUP_ = 8,
            SLIVERUP_ = 10,
            PAWNUP_ = 11 };

        enum player_{
            LOWER_ = 0,
            UPPER_ = 1 };
};
