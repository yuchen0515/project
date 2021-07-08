#include <iostream>
#include <vector>
#include <cassert>

#include "state.h"
#include "exportHistory.h"

#define AGENT

// 初始化 (無走步狀態)
const Move State::no_move = {
    std::make_pair(-1, -1),
    std::make_pair(-1, -1)};

// Declare (避免 compiler error, mcts.h 已定義)
namespace PURE {
    Move MCTS(State state, int32_t simLimit);
}


// Agent setting
Move State::Agent(
        const int32_t lowerSimul,
        const int32_t upperSimul) {

    auto STATE = (*this);
    int32_t simul = 0;

    // 上下手 Agent 模擬次數分別設定
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

    // 檢查邊界
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

// 移動走步 (於介面)
void State::do_Move(Move move) {
    auto &[a, b] = move.from;
    auto &[c, d] = move.to;

    //std::cout << a << " " << b << std::endl;
    //std::cout << c << " " << d << std::endl;

#ifdef PRINT
    std::cout << a << " " << b << " " << c << " " << d << std::endl;
#endif

    // 超出邊界
    if ((a < 0 || a > 4)
            || (b < -2 || b > 6)
            || (c < 0 || c > 4)
            || (d < 0 && d > 4)) {
        std::cerr << "Error: " << __FUNCTION__ << std::endl;
        return;
    }

    bool isDesKing = false;
    int32_t ori_turn = get_turns();
    if (int32_t opponent = (get_turns() == 1 ? 0 : 1);
            exist[opponent][c][d] == KING_) {
        isDesKing = true;
    }

    // 移動棋子(資料結構內)
    MoveChess(move.from, move.to);

    if (isDesKing == true && exist[ori_turn == 0 ? 1 : 0][c][d] > 0) {
        isKingDead_ = true;
    }

    //turn_ = 1 - turn_;
    //player_to_move_ = turn_;
    //player_to_move_ = get_turns();

    return;
}

bool State::is_Draw() const {
    return false;
}

// Game 運行
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

    //init
    game_number_ = 25;

    std::vector<int32_t> simula_TEMP = {
        150, //UPPER
        //50000,  // Upper
      150}; // Lower

    // 對戰選擇
    std::cout << "------Game Setting------" << std::endl;
    std::cout << "Round: ";
    std::cin >> game_number_;
    std::cout << "Simulation(Lower): ";
    std::cin >> simula_TEMP[LOWER_];
    std::cout << "Simulation(Upper): ";
    std::cin >> simula_TEMP[UPPER_];
    std::cout << "---" << std::endl;
    std::cout << "<Type>" << std::endl;
    std::cout << "  | Lower  |  Upper |" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "1 | Player | Player |" << std::endl;
    std::cout << "2 | Agent  | Player |" << std::endl;
    std::cout << "3 | Player | Agent  |" << std::endl;
    std::cout << "4 | Agent  | Agent  |" << std::endl;
    std::cout << "---------------------" << std::endl;
    int32_t type = 0;
    while (type <= 0 || type > 4) {
        std::cout << "> ";
        std::cin >> type;
    }

    if (type == 2) {
        player_setting_[LOWER_] = PLAYER_TYPE_::AGENT_;
    } else if (type == 3) {
        player_setting_[UPPER_] = PLAYER_TYPE_::AGENT_;
    } else if (type == 4) {
        player_setting_[LOWER_] = PLAYER_TYPE_::AGENT_;
        player_setting_[UPPER_] = PLAYER_TYPE_::AGENT_;
    }


    //
    std::cout << "------Game Information------" << std::endl;
    History stepRecord;
    stepRecord.getCurrentSystemTime();
    stepRecord.openFile();

    while (quit == false && game_number_ > 0) {
        // Clear screen
        SDL_RenderClear(gRenderer);

        // cover
        show_walking(mouseIndex_);

        // 電腦 vs 電腦 檢查
        int32_t SelfPlay =  ((player_setting_[LOWER_] == PLAYER_TYPE_::AGENT_) && (player_setting_[UPPER_] == PLAYER_TYPE_::AGENT_));
        bool agentCheck = false;
        if (player_setting_[get_turns()] == PLAYER_TYPE_::AGENT_) {
            agentCheck = true;
        }

        // 自我對下 處理走步  
        if (agentCheck == true && agentDone_ == false && isKingDead_ == false){
            auto TEMP = Agent(simula_TEMP[0], simula_TEMP[1]);
            agentDone_ = true;
            make_walking(TEMP.from, this->walking);
            stepRecord.recordStep(TEMP.from, TEMP.to);
            MoveChess(TEMP.from, TEMP.to);
        }

        //selfplay
        // 自我對下 判斷結束
        if (SelfPlay && isKingDead_ == true) {
            InitExist();
            isKingDead_ = false;
            lose_[get_turns()] += 1;
            winner_[get_turns() == 1 ? 0 : 1] += 1;
            game_number_ -= 1;
            std::cerr << "Count: " << game_number_ << std::endl;
            std::cerr << "Winner: "; 

            if (get_turns() == 0) {
                std::cerr << "Upper" << std::endl;

            }else {
                std::cerr << "Lower" << std::endl;
            }
            std::cerr << std::endl;
        }

        // 偵測 & 無限迴圈跑介面
        while ((SDL_PollEvent(&e) && agentDone_ == false)) {
            // 離開
            if (e.type == SDL_QUIT
                    || e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }

            // 重整
            if (e.key.keysym.sym == SDLK_a) {
                InitExist();
                isKingDead_ = false;
            }

            // 王死亡、畫面停止
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
                        stepRecord.recordStep(mouseIndexTemp_, mouseIndex_);
                        MoveChess(mouseIndexTemp_, mouseIndex_);
                        isClickChess_ = false;

                        if (DetectKingExist() == 0) {
                            isKingDead_ = true;
                            lose_[get_turns()] += 1;
                            winner_[get_turns() == 1 ? 0 : 1] += 1;
                            game_number_ -= 1;
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
                    // 點擊檢查
                    if (isClickChess_
                            && (mFir != mFirTEMP
                                || mSec != mSecTEMP)) {
                        stepRecord.recordStep(mouseIndexTemp_, mouseIndex_);
                        MoveChess(mouseIndexTemp_, mouseIndex_);
                        isClickChess_ = false;

                        if (DetectKingExist() == 0) {
                            lose_[get_turns()] += 1;
                            winner_[get_turns() == 1 ? 0 : 1] += 1;
                            game_number_ -= 1;
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
#ifdef PRINT
                std::cerr << "index_x: " << mFir << ", ";
                std::cerr << mSec << std::endl;
#endif

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

    // 對局統計輸出
    if (winner_[UPPER_] > winner_[LOWER_]){
        std::cout << "Winner: Upper" << std::endl;
    } else if (winner_[UPPER_] == winner_[LOWER_]){
        std::cout << "Winner: Draw" << std::endl;
    } else if (winner_[UPPER_] < winner_[LOWER_]){
        std::cout << "Winner: Lower" << std::endl;
    }

    std::cout << "Upper[";
    if (player_setting_[UPPER_] == PLAYER_TYPE_::PLAYER_) {
        std::cout << "Player";
    } else {
        std::cout << "Agent] Simulate: " << simula_TEMP[UPPER_];
    }

    std::cout << std::endl;
    std::cout << "Win: " << winner_[UPPER_];
    std::cout << ", Lose: " << lose_[UPPER_];
    std::cout << ", Win rate: " << 1.0 * winner_[UPPER_] / (lose_[UPPER_] + winner_[UPPER_]) << std::endl;

    std::cout << "---" << std::endl;

    std::cout << "Lower[";
    if (player_setting_[LOWER_] == PLAYER_TYPE_::PLAYER_) {
        std::cout << "Player]";
    } else {
        std::cout << "Agent] Simulate: " << simula_TEMP[LOWER_];
    }

    std::cout << std::endl;
    std::cout << "Win: " << winner_[LOWER_];
    std::cout << ", Lose: " << lose_[LOWER_];
    std::cout << ", Win rate: " << 1.0 * winner_[LOWER_] / (lose_[LOWER_] + winner_[LOWER_]) << std::endl;

    stepRecore.closeFile();

    close();
}

// sigmoid 處理審局函數
double State::sigmoid(const double score) const {
    std::cerr << "turn" << get_turns() << " :";

    if (score >= 10000) {
        std::cerr << "1.0" << std::endl;
        return 1.0;
    } else if (score <= -10000) {
        std::cerr << "-1.0" << std::endl;
        return -1.0;
    }

    std::cerr << 2 * ((1.0 / (1.0 + exp(-1 * (step_tangent(score))))) - 0.5) << std::endl;
    return 2 * ((1.0 / (1.0 + exp(-1 * (step_tangent(score))))) - 0.5);
}

// 斜率調整
double State::step_tangent(const double score) const {
    // blank
    double lower_number = 84.0;
    lower_number /= 1.3;

    return score / lower_number;
}


// 取得盤面分數
double State::get_board_score(const int32_t player_type) const {
    double res = 0.0;
    for (int32_t y = 0 ; y < ROW_SIZE_ + 2 ; y++) {
        for (int32_t x = 0 ; x < COL_SIZE_; x++) {
            res += evl_value(player_type, std::make_pair(x,y));
        }
    }
    return res;
}


// 換對局方
void State::switch_turn() {
    turn_ = (PLAYER_NUMBER_ - 1) - turn_;
}
