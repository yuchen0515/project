#include <iostream>
#include <vector>

#include "state.h"

#define AGENT

const Move State::no_move = {
    std::make_pair(-1, -1),
    std::make_pair(-1, -1)};

namespace PURE {
    Move MCTS(State state, int32_t simLimit);
}

Move State::Agent(
        const int32_t lowerSimul,
        const int32_t upperSimul) {

    auto STATE = (*this);
    int32_t simul = 0;

    if (get_turns() == LOWER_){
        simul = lowerSimul;
    } else if (get_turns() == UPPER_){
        simul = upperSimul;
    }

    auto TEMP = PURE::MCTS(STATE, simul);

    auto &[a, b] = TEMP.from;
    auto &[c, d] = TEMP.to;

    a = (a < 0 || a > 4) ? 0 : a;
    b = (b < -2 || b > 6) ? 0 : b;
    c = (c < 0 || c > 4) ? 0 : c;
    d = (d < 0 || d > 4) ? 0 : d;

    assert(a >= 0 && a <= 4);
    assert(b >= -2 && b <= 6);

    assert(c >= 0 && c <= 4);
    assert(d >= 0 && d <= 4);

    return TEMP;
}

//void State::set_State() {
//}
//void State::set_Result() {
//}
//Result State::get_Result() const {
//}
//bool State::is_End() const {
//}
//int32_t State::get_turns() const {
//}
//std::vector<Move> State::get_Moves() const {
//}

void State::do_Move(Move move) {
    auto &[a, b] = move.from;
    auto &[c, d] = move.to;

    //std::cout << a << " " << b << std::endl;
    //std::cout << c << " " << d << std::endl;

    std::cout << a << " " << b << " " << c << " " << d << std::endl;

    if ((a < 0 || a > 4)
            || (b < -2 || b > 6)
            || (c < 0 || c > 4)
            || (d < 0 && d > 4)) {
        std::cerr << "Error: " << __FUNCTION__ << std::endl;
        return;
    }

    if (int32_t oppoent = (get_turns() == 1 ? 0 : 1) ;
            exist[oppoent][c][d] == KING_){
        isKingDead_ = true;
    }

    MoveChess(move.from, move.to);

    return;
}

bool State::is_Draw() const {
    return false;
}

void State::run(){
    // Start up SDL and create window
    if (init() == false) {
        std::cerr << "Failed to initialize!" << std::endl;
        return;
    }
    InitMedia();

    // Load media
    if (loadMedia(&gTextureBoard,
                const_cast<char *> ("../image/board.bmp")) == false) {
        std::cerr << "Failed to load media!" << std::endl;
        return;
    }
    // Update the surface
    SDL_UpdateWindowSurface(window);

    // Wait two seconds
    SDL_Event e;

    bool quit = false;

    while (quit == false) {
        // Clear screen
        SDL_RenderClear(gRenderer);

        // cover
        show_walking(mouseIndex_);

        //
        player_setting_[LOWER_] = PLAYER_TYPE_::AGENT_;

        bool agentCheck = false;
        if (player_setting_[get_turns()] == PLAYER_TYPE_::AGENT_) {
            agentCheck = true;
        }

        if (agentCheck == true && agentDone_ == false && isKingDead_ == false){
            auto TEMP = Agent(100, 1000);
            agentDone_ = true;
            make_walking(TEMP.from, this->walking);
            MoveChess(TEMP.from, TEMP.to);
        }

        while (SDL_PollEvent(&e) && agentDone_ == false) {
            if (e.type == SDL_QUIT
                    || e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }

            if (e.key.keysym.sym == SDLK_a) {
                InitExist();
                isKingDead_ = false;
            }

            if (isKingDead_ == true) {
                usleep(50000);
                continue;
            }

            auto &[mFir, mSec] = mouseIndex_;
            auto &[mFirTEMP, mSecTEMP] = mouseIndexTemp_;

            if (SDL_MOUSEBUTTONDOWN == e.type) {
                if (SDL_BUTTON_LEFT == e.button.button) {
                    mouseIndexTemp_ = mouseIndex_;
                    mouseX_ = e.button.x;
                    mouseY_ = e.button.y;
                    mouseIndex_ = return_MouseIndex(mouseX_, mouseY_);
#ifdef PRINT
                    std::cerr << "x, y: " << mouseX_;
                    std::cerr << " " << mouseY_;
                    std::cerr << "..............." << std::endl;
#endif

                    if (isClickChess_
                            && (mFir != mFirTEMP
                                || mSec != mSecTEMP)) {
                        MoveChess(mouseIndexTemp_, mouseIndex_);
                        isClickChess_ = false;

                        if (DetectKingExist() == 0) {
                            isKingDead_ = true;
                        }
                    } else if (ClickCover(mouseIndex_)
                            && isClickChess_ == true
                            && (mFir == mFirTEMP
                                && mSec == mSecTEMP)) {
                        isClickChess_ = false;
                    } else {
                        isClickChess_ = ClickCover(mouseIndex_) ? 1 : 0;
                    }
                } else if (SDL_BUTTON_RIGHT == e.button.button) {
                    mouseIndexTemp_ = mouseIndex_;
                    mouseX_ = e.button.x;
                    mouseY_ = e.button.y;
                    mouseIndex_ = return_MouseIndex(mouseX_, mouseY_);
#ifdef PRINT
                    std::cerr << "x, y: " << mouseX_;
                    std::cerr << " " << mouseY_;
                    std::cerr << "..............." << std::endl;
#endif
                    if (isClickChess_
                            && (mFir != mFirTEMP
                                || mSec != mSecTEMP)) {
                        MoveChess(mouseIndexTemp_, mouseIndex_);
                        isClickChess_ = false;

                        if (DetectKingExist() == 0) {
                            isKingDead_ = true;
                        }
                    } else if (ClickCover(mouseIndex_)
                            && isClickChess_ == true
                            && (mFir == mFirTEMP
                                && mSec == mSecTEMP)) {
                        isClickChess_ = false;
                    } else {
                        isClickChess_ = ClickCover(mouseIndex_) ? 1 : 0;
                    }
                }
                std::cerr << "index_x: " << mFir << ", ";
                std::cerr << mSec << std::endl;

                PrintBugMessageBoard();
            }
        }
        agentDone_ = false;
        // Render texture to screen
        SDL_RenderCopy(gRenderer, gTextureBackground, NULL, &texPosition_[0]);
        // Apply the image
        SDL_RenderCopy(gRenderer, gTextureBoard, NULL, &texPosition_[1]);

        if (isClickChess_ == false) {
            show_walking(std::make_pair(-1, -1));
        } else {
            show_walking(mouseIndex_);
        }

        Show_Chess();
        ClickCover(mouseIndex_);

        // Update screen
        SDL_RenderPresent(gRenderer);
    }
    close();
}
