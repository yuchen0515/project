#include <iostream>
#include <vector>
#include <cassert>

#include "state.h"

const Move State::no_move = {std::make_pair(-1, -1), std::make_pair(-1, -1)};

namespace PURE {
    Move MCTS(State state, int32_t simLimit);
}

Move State::Agent(){

    State state = *this;
    int32_t simul = 0;

    if (get_turns() == 0){
        simul = 1000;
    }else{
        simul = 100;
    }
    auto TEMP = PURE::MCTS(state, simul);
    //std::cout << "hihihi" << std::endl;

    auto &[a, b] = TEMP.from;
    //std::cout << a << " " << b << std::endl;
    auto &[c, d] = TEMP.to;
    //std::cout << c << " " << d << std::endl;

    //std::cout << "TTTTTTTTTTTTTTT----" << std::endl;
    
    if (a < 0 || a > 4){
        a = 0;
    }
    if (b < 0 || b > 4){
        b = 0;
    }
    if (c < 0 || c > 4){
        c = 0;
    }
    if (d < 0 || d > 4){
        d = 0;
    }

    assert(a >= 0 && a <= 4);
    assert(b >= -2 && b <= 6);

    assert(c >= 0 && c <= 4);
    assert(d >= 0 && d <= 4);
    return TEMP;
    MoveChess(TEMP.from, TEMP.to);
}

//void State::set_State() {
//
//}
//
//void State::set_Result() {
//
//}
//
//Result State::get_Result() const {
//    return result_;
//}
//
//bool State::is_End() const {
//    return false;
//}
//
//
//int32_t State::get_turns() const {
//    return 0;
//}
//
//std::vector<Move> State::get_Moves() const {
//    return {};
//}
//
void State::do_Move(Move move) {
    auto &[a, b] = move.from;
    //std::cout << a << " " << b << std::endl;
    auto &[c, d] = move.to;
    //std::cout << c << " " << d << std::endl;

    std::cout << a << " " << b << " " << c << " " << d << std::endl;

    assert(a >= 0 && a <= 4);
    assert(b >= -2 && b <= 6);

    assert(c >= 0 && c <= 4);
    assert(d >= 0 && d <= 4);

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
    } else {
        InitMedia();

        // Load media
        if (loadMedia(&gTextureBoard,
                    const_cast<char *> ("../image/board.bmp")) == false) {
            std::cerr << "Failed to load media!" << std::endl;
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
                show_walking(mouseIndex_);
                //if (get_turns() == 0 && agentDone_ == false && isKingDead_ == false){
                //if (get_turns() == 1 && agentDone_ == false && isKingDead_ == false){
                //if (get_turns() == 0 && agentDone_ == false && isKingDead_ == false){
                //    auto TEMP = Agent();
                //    agentDone_ = true;
                //    make_walking(TEMP.from, this->walking);
                //    MoveChess(TEMP.from, TEMP.to);
                //}

                while (SDL_PollEvent(&e) && agentDone_ == false) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    if (e.key.keysym.sym == SDLK_ESCAPE) {
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
        }
    }
    close();
}
