/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學 National Taiwan Normal University
 * 資訊工程學系 Computer Science and Information Engineering
 */

#include "interface.h"
//#define PRINT

void Interface::Agent(){
    //State state;
    //auto TEMP = PURE::MCTS(state, 1000);
    //MoveChess(TEMP.from, TEMP.to);
}

int32_t Interface::get_turns() const {
    return turn_;
}

bool Interface::is_End() const {
    return isKingDead_ == true;
}

std::vector<Move> Interface::get_Moves()  const {
    std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
    //move_.clear();
    std::vector<std::vector<int32_t>> walking(COL_SIZE_, std::vector<int32_t>(ROW_SIZE_, 0));

    std::vector<Move> move__;

    Move TEMP;
    for (int32_t i = 0 ; i < 5 ; i++){
        for (int32_t j = 0 ; j < 7 ; j ++) {
            TEMP.from = std::make_pair(i, j);
            make_walking(TEMP.from, walking);
            for (int32_t k = 0 ; k < ROW_SIZE_ ; k ++){
                for (int32_t p = 0 ; p < COL_SIZE_ ; p ++){
                    if (walking[k][p] == 1){
                        TEMP.to = std::make_pair(k, p);
                        //TEMP.value = evl_value(get_turns() == 1 ? 0 : 1, TEMP.to) - evl_value(get_turns(), TEMP.from) * 0.9;
                        TEMP.value = evl_value(get_turns() == 1 ? 0 : 1, TEMP.to);
                        //
                        //if (exist[get_turns()][i][j] == KING_) {
                        //    TEMP.value += 30000;
                        //}
                        //
                        //TEMP.value -= 1.2 * evl_value(get_turns(), TEMP.from);
                        move__.emplace_back(TEMP);
                    }
                }
            }
        }
    }
    struct cmp{
        bool operator()(const Move& a, const Move& b){
            return a.value > b.value;
            //if (a.value > b.value){
            //    return a.value > b.value;
            //} 
            //else if (a.value == b.value){
            //    return rand() % 2 == 0;
            //}
            //return false;
        }
    };

    std::uniform_real_distribution<long double> rf(0, 1);

    shuffle(move__.begin(), move__.end(), rng);
    //sort(move__.begin(), move__.end(), cmp());
    std::vector<Move> SA_move;

    int32_t max_span = move__[0].value;
    SA_move.emplace_back(move__[0]);

    //SA selection
    for (int32_t i = 1 ; i < move__.size(); i++) {
        if (bool isMyKing = exist[get_turns()][move__[i].from.first][move__[i].from.second] == KING_ ;
                move__[i].value >= max_span || isMyKing == true){
            //if (isMyKing == false) {
                max_span = move__[i].value;
            //}
            SA_move.emplace_back(move__[i]);
        } else if (std::exp((move__[i].value - max_span + 0.0) / 1e2) > rf(rng)) {
            SA_move.emplace_back(move__[i]);
            //max_span = move__[i].value;
        }
    }

    return SA_move;
    return move__;
}

int32_t Interface::evl_value(int32_t turn, std::pair<int32_t, int32_t> temp)const {
    auto comp = exist[turn][temp.first][temp.second];

    switch (comp) {
        case KING_:
            return 30000;
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


void Interface::InitMedia() {
    loadMedia(
            &gTextureBackground,
            const_cast<char *> ("../image/background.bmp"));
    loadMedia(
            &gTextureKing,
            const_cast<char *> ("../image/king.bmp"));
    loadMedia(
            &gTextureJade,
            const_cast<char *> ("../image/jade.bmp"));
    loadMedia(
            &gTextureGold,
            const_cast<char *> ("../image/gold.bmp"));
    loadMedia(
            &gTextureSliver,
            const_cast<char *> ("../image/sliver.bmp"));
    loadMedia(
            &gTextureSliverUp,
            const_cast<char *> ("../image/sliverup.bmp"));
    loadMedia(
            &gTextureBishop,
            const_cast<char *> ("../image/bishop.bmp"));
    loadMedia(
            &gTextureBishopUp,
            const_cast<char *> ("../image/bishopup.bmp"));
    loadMedia(
            &gTexturePawn,
            const_cast<char *> ("../image/pawn.bmp"));
    loadMedia(
            &gTexturePawnUp,
            const_cast<char *> ("../image/pawnup.bmp"));
    loadMedia(
            &gTextureRook,
            const_cast<char *> ("../image/rook.bmp"));
    loadMedia(
            &gTextureRookUp,
            const_cast<char *> ("../image/rookup.bmp"));
    loadMedia(
            &gTextureAlphaChess,
            const_cast<char *> ("../image/alphachess.bmp"));
    loadMedia(
            &gTextureLatticeCover,
            const_cast<char *> ("../image/latticecover.bmp"));
}

void Interface::InitExist() {
    //memset(exist, 0, sizeof(exist));
    std::fill(exist.begin(), exist.end(), std::vector<std::vector<int32_t>>(COL_SIZE_, std::vector<int32_t>(7, 0)));

    exist[0][0][0] = exist[1][4][4] = ROOK_;
    exist[0][1][0] = exist[1][3][4] = BISHOP_;
    exist[0][2][0] = exist[1][2][4] = SLIVER_;
    exist[0][3][0] = exist[1][1][4] = GOLD_;
    exist[0][4][0] = exist[1][0][4] = KING_;
    exist[0][4][1] = exist[1][0][3] = PAWN_;

    std::fill(walking.begin(),
            walking.end(),
            std::vector<int32_t>(ROW_SIZE_, 0));

    mouseIndex_ = std::make_pair(-1, -1);

    turn_ = (turn_ + 1) % 2;
}

bool Interface::DetectKingExist() const {
    int32_t exist_king = 0;
    for (int32_t i = UPPER_; i <= LOWER_; i++) {
        for (int32_t j = 0 ; j < COL_SIZE_ ; j++) {
            for (int32_t k = 0 ; k < ROW_SIZE_ ; k++) {
                if (exist[i][j][k] == 1)
                    exist_king += 1;
            }
        }
    }

    if (exist_king == 2) {
        return true;
    }

    return false;
}

void Interface::InitPosition() {
    setup_bmp_size(&texPosition_[0],
            0,
            0,
            SCREEN_WIDTH * 1.2,
            SCREEN_HEIGHT);

    setup_bmp_size(&texPosition_[1],
            140,
            140,
            380,
            380);

    for (int32_t i = 0 ; i < 5 ; i++) {
        for (int32_t j = 0 ; j < 5 ; j++) {
            setup_bmp_size(
                    &chessDect_[i][j],
                    155 + 77 * i,
                    152 + 75 * j,
                    125,
                    250);
        }
    }

    //Upper
    for (int32_t i = 0 ; i <= 5 ; i++) {
        for (int32_t j = 5 ; j <= 6 ; j++) {
            setup_bmp_size(
                    &chessDect_[i][j],
                    155 + 77 * i,
                    152 - 75 * (j - 4),
                    125,
                    250);
        }
    }

    //Lower
    for (int32_t i = 0 ; i <= 5 ; i++) {
        for (int32_t j = 7 ; j <= 8 ; j++) {
            setup_bmp_size(
                    &chessDect_[i][j],
                    155 + 77 * i,
                    152 + 75 * (j - 2),
                    125,
                    250);
        }
    }
}

bool Interface::init() {
    InitExist();
    InitPosition();

    // The window we'll be rendering to
    // The surface contained by the window
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << ("SDL couldn't initialize!") << std::endl;
        std::cout << "SDL_Error detail: ";
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    // Set texture filtering to linear
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == false) {
        std::cout << "Warning: Linear texture ";
        std::cout << "filtering not enabled!" << std::endl;
    }

    // Create window
    window = SDL_CreateWindow(
            "Pioneer",                   // Title
            SDL_WINDOWPOS_CENTERED,         // X position setting
            SDL_WINDOWPOS_CENTERED,         // Y position setting
            SCREEN_WIDTH,                   // Width
            SCREEN_HEIGHT,                  // Height
            SDL_WINDOW_SHOWN);              // Show window

    if (window == nullptr) {
        std::cout << "Window couldn't be created!" << std::endl;
        std::cout << "SDL_Error: ";
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer for window
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (gRenderer == nullptr) {
        std::cout << "Renderer could not be created!" << std::endl;
        std::cout << "SDL_Error: ";
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize renderer color
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);

    return true;
}

void Interface::CaptivePush(const int32_t kind, int32_t chess) {
    if (chess >= ROOKUP_ && chess <= PAWNUP_) {
        chess -= LEVEL_CHANGE_;
    }

    static constexpr int32_t CaptiveRowSize = 2;
    for (int32_t j = 0 + ROW_SIZE_ ; j < CaptiveRowSize + ROW_SIZE_; j++) {
        for (int32_t i = 0 ; i < COL_SIZE_; i++) {
            if (exist[kind][i][j] == 0){
                exist[kind][i][j] = chess;
                return;
            }
        }
    }
}

void Interface::MoveChess(
        std::pair<int32_t, int32_t> ori,
        std::pair<int32_t, int32_t> des) {


    auto &[ori_a, ori_b] = ori;
    auto &[des_a, des_b] = des;

    bool isCaptive = false;
    bool isUpperCaptive = false;

    if (isWithinBound(ori) == false) {
        return;
    }

    if (des_a < 0 || des_a > 4 || des_b < 0 || des_b > 4) {
        return;
    }

    //captive
    if (ori_b < 0 && get_turns() == 1) {
        return ;
    }
    if (ori_b > 4) {
        isCaptive = true;
        isUpperCaptive = true;
    }
    if (ori_b < 0) {
        ori_b += 7;
        ori_b = 11 - ori_b;
        isCaptive = true;
    }

    int32_t kind = (get_turns() == 0 && exist[0][ori_a][ori_b]) > 0 ? 0 : 1;
    int32_t kind_des = exist[0][des_a][des_b] > 0 ? 0 : 1;
    int32_t IsChess = 0;
    int32_t walk_check = walking[des_a][des_b];


#ifdef PRINT
    std::cout << "ori: " << ori_a << " " << ori_b << std::endl;
    std::cout << "des: " << des_a << " " << des_b << std::endl;
    std::cout << "kind: " << kind << ", " << kind_des << std::endl;
    std::cout << "walk_check: " << walking[des_a][des_b] << std::endl;
#endif

    int32_t oppoent = (kind == 1 ? 0 : 1);

    //No chess
    if (kind == 1 && exist[1][ori.first][ori.second] == 0) {
        return;
    }

    //No Move
    if (walk_check == 0) {
        return;
    }

    //Destination having a opponent chess
    if (exist[oppoent][des.first][des.second] > 1) {
        IsChess = 1;
    }

    //No move to myself chess
    if (kind_des == 0 && kind == 0) {
        return;
    }

    //NOT from captive move to a chess rect.
    if (IsChess == 1 && isCaptive == true) {
        return;
    }

    //Push chess to captive blocks
    if (IsChess == 1) {
        auto& [fir, sec] = des;
        CaptivePush(kind, exist[kind == 1 ? 0 : 1][fir][sec]);
    }

    bool isDesKing = false;
    //King Dead is or not
    if (exist[oppoent][des.first][des.second] == KING_){
        isDesKing = true;
    }

    exist[kind][des_a][des_b]= exist[kind][ori_a][ori_b];
    exist[kind][ori_a][ori_b] = 0;
    exist[oppoent][des_a][des_b] = 0;

    if (isDesKing == true && exist[oppoent][des.first][des.second] == 0){
        isKingDead_ = true;
    }


    if ((ori_b >= 0 && ori_b <= 4)
            && (des_b == 4 * oppoent)) {
        //All chess that can level up
        if (int32_t chess = exist[kind][des_a][des_b]; 
                (chess >= ROOK_
                 && chess <= PAWN_)
                && chess != GOLD_) {
            exist[kind][des_a][des_b] += LEVEL_CHANGE_;
        }
    }

#ifdef PRINT
    std::cout << "Kind: " << kind << std::endl;
    std::cout << "ori_x: " << ori_a << " ori_y: " << ori_b << std::endl;
    std::cout << "des_x: " << des_a << " des_y: " << des_b << std::endl;
#endif

    //
    //if (isRepeat()) {
    //    std::cout << "th........" << std::endl;
    //}
    //

    //std::cout << "hi : " << player_to_move_ << std::endl;

    turn_ = (PLAYER_NUMBER_ - 1) - turn_;
    player_to_move_ = 1 - player_to_move_;
}

void Interface::PrintBugMessageBoard() const {
    //#ifndef PRINT
    //system("clear");
    //#endif
    for (int32_t k = 0 ; k < 2 ; k++) {
        std::cout << "------------" << std::endl;
        for (int32_t i = 0 ; i < 7 ; i ++) {
            for (int32_t j = 0 ; j < 5 ; j ++) {
                if (j != 0){
                    std::cout << "\t";
                }
                std::cout << exist[k][j][i];
            }
            std::cout << std::endl;
        }
    }
    std::cout << "------------" << std::endl;
}

void Interface::Determine_Draw(const int32_t kind, const int32_t Isupper, const int32_t j, const int32_t k) {
    switch (kind) {
        case KING_:
            if (Isupper) {
                gTextureShow = gTextureKing;
            } else if (Isupper == 0) {
                gTextureShow = gTextureJade;
            }
            break;
        case ROOK_:
            gTextureShow = gTextureRook;
            break;
        case BISHOP_:
            gTextureShow = gTextureBishop;
            break;
        case GOLD_:
            gTextureShow = gTextureGold;
            break;
        case SLIVER_:
            gTextureShow = gTextureSliver;
            break;
        case PAWN_:
            gTextureShow = gTexturePawn;
            break;
        case ROOKUP_:
            gTextureShow = gTextureRookUp;
            break;
        case BISHOPUP_:
            gTextureShow = gTextureBishopUp;
            break;
        case SLIVERUP_:
            gTextureShow = gTextureSliverUp;
            break;
        case PAWNUP_:
            gTextureShow = gTexturePawnUp;
            break;

        default:
            gTextureShow = nullptr;
    }

    if (gTextureShow != nullptr) {
        if (Isupper == 1) {
            SDL_RenderCopyEx(
                    gRenderer,
                    gTextureShow,
                    nullptr,
                    &chessDect_[j][k],
                    180,
                    &chessSize_,
                    SDL_FLIP_NONE);
        } else if (Isupper == 0) {
            if (k < 5) {
                SDL_RenderCopy(
                        gRenderer,
                        gTextureShow,
                        nullptr,
                        &chessDect_[j][k]);
            } else if (k < 8) {
                SDL_RenderCopy(
                        gRenderer,
                        gTextureShow,
                        nullptr,
                        &chessDect_[j][k+2]);
            }
        }
    }
}

bool Interface::ClickCover(const std::pair<int32_t, int32_t> fMouseIndex) const {
    if (auto [fir, sec] = fMouseIndex;
            fir >= 0
            && fir <= 4
            && sec >= -2
            && sec <= 6) {
        bool isUpperCaptive = false;

        //Upper Captive
        if (sec < 0) {
            sec -= 4;
            sec *= (-1);
            isUpperCaptive = true;
        }

        //Lower
        if (isUpperCaptive == false && exist[1][fir][sec] > 0) {
            //Lower Captive
            if (sec >= 5) {
                sec += 2;
            }
            SDL_RenderCopy(
                    gRenderer,
                    gTextureAlphaChess,
                    nullptr,
                    &chessDect_[fir][sec]);
        } 
        //Upper
        if ( exist[0][fir][sec] > 0) {
            SDL_RenderCopyEx(
                    gRenderer,
                    gTextureAlphaChess,
                    nullptr,
                    &chessDect_[fir][sec],
                    180,
                    &chessSize_, SDL_FLIP_NONE);
        }
        return true;
    }
    return false;
}

void Interface::Show_Chess() {
    for (int32_t i = 0 ; i < 2 ; i++) {
        for (int32_t j = 0 ; j < 5 ; j++) {
            for (int32_t k = 0 ; k < 7 ; k++) {
                Determine_Draw(exist[i][j][k], (1-i), j, k);
            }
        }
    }
}

void Interface::make_walking(
        const std::pair<int32_t, int32_t> temp,
        std::vector<std::vector<int32_t>>& walking) const {

    std::fill(walking.begin(),
            walking.end(),
            std::vector<int32_t>(ROW_SIZE_, 0));

    int32_t Isupper = 0, kind = 0;
    auto [fir, sec] = temp;

    if (isWithinBound(temp) == false){
        return;
    }

    bool isCaptive = false;
    bool isUpperCaptive = false;

    //Upper Captive
    if (sec < 0) {
        sec += 7;
        sec = 11 - sec;
        isCaptive = true;
        isUpperCaptive = true;

    }

    //Lower Captive
    if (sec > 4) {
        isCaptive = true;
    }

    if (exist[0][fir][sec] > 0) {
        Isupper = 1;
        kind = exist[0][fir][sec];
    } 
    if (isUpperCaptive == false && exist[1][fir][sec] > 0) {
        Isupper = 0;
        kind = exist[1][fir][sec];
    }

    //No chess
    if (kind == 0) {
        return;
    }

    //No your turns
    if (get_turns() == Isupper){
        return;
    }


    if (isCaptive == true) {
        for (int32_t x = 0 ; x < COL_SIZE_ ; x ++) {
            int32_t count_PAWN = 0;
            for (int32_t y = 0 ; y < ROW_SIZE_; y++ ) {
                if (exist[0][x][y] == 0 && exist[1][x][y] == 0) {
                    walking[x][y] = 1;
                }
                count_PAWN += (exist[get_turns()][x][y] == PAWN_);
                //count_PAWN += (exist[1][x][y] == PAWN_);
            }

            if (count_PAWN == 1 && kind == PAWN_) {
                for (int32_t y = 0 ; y < ROW_SIZE_; y++ ) {
                    walking[x][y] = 0;
                }
            }

            if (int32_t Opponent_y = (Isupper == 1) ? 4 : 0 ;
                    kind == PAWN_) {

                for (int32_t x = 0 ; x < COL_SIZE_ ; x++) {
                    walking[x][Opponent_y] = 0;
                }
            }
        }
        return;
    }

    int32_t direction = (Isupper == 1) ? 1 : -1;

    //Left Top
    if (check_bound_xy(fir, sec, -1, direction, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == SLIVER_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir - 1][sec + direction] = 1;
        }
    }

    //Top
    if (check_bound_xy(fir, sec, 0, direction, Isupper)) {
        if (kind == KING_
                || (kind >= GOLD_ && kind <= BISHOPUP_)
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir][sec + direction] = 1;
        }
    }

    //Right Top
    if (check_bound_xy(fir, sec, 1, direction, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == SLIVER_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir + 1][sec + direction] = 1;
        }
    }

    //Left
    if (check_bound_xy(fir, sec, -1, 0, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir - 1][sec] = 1;
        }
    }

    //Right
    if (check_bound_xy(fir, sec, 1, 0, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir + 1][sec] = 1;
        }
    }

    //Left down
    if (check_bound_xy(fir, sec, -1, -1 * direction, Isupper)) {
        if (kind == KING_
                || kind == SLIVER_
                || kind == ROOKUP_
                || kind == BISHOPUP_) {
            walking[fir - 1][sec - direction] = 1;
        }
    }

    //down
    if (check_bound_xy(fir, sec, 0, -1 * direction, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir][sec - direction] = 1;
        }
    }

    //Right Down
    if (check_bound_xy(fir, sec, 1, -direction, Isupper)) {
        if (kind == KING_
                || kind == SLIVER_
                || kind == ROOKUP_
                || kind == BISHOPUP_) {
            walking[fir + 1][sec - direction] = 1;
        }
    }

    static const int32_t dec[8][2] = {
        {-1 , direction},
        {0, direction},
        {1, direction},
        {1, 0},
        {1, -1 * direction},
        {0, -1 * direction},
        {-1, -1 * direction},
        {-1, 0}
    };

    if (kind != ROOK_ 
            && kind != BISHOP_
            && kind != ROOKUP_
            && kind != BISHOPUP_) {
        return;
    }


    for (int32_t i =
            (kind == BISHOP_
             || kind == BISHOPUP_) ? 0 : 1;
            i < BISHOPUP_ ;
            i += 2) {
        int32_t level = 1;

        while (true) {
            if (check_bound_xy(
                        fir,
                        sec,
                        dec[i][0] * level,
                        dec[i][1] * level,
                        Isupper) == 0) {
                break;
            }


            walking[fir + dec[i][0] * level][sec + dec[i][1] * level] = 1;

            if (check_bound_xy(
                        fir,
                        sec,
                        dec[i][0] * level,
                        dec[i][1] * level,
                        Isupper) == 2) {
                break;
            }

            level += 1;
        }
    }
}

void Interface::show_walking(
        const std::pair<int32_t, int32_t> temp) {
    make_walking(temp, this->walking);
    for (int32_t i = 0 ; i < 5 ; i ++){
        for (int32_t j = 0 ; j < 5 ; j++){
            if (walking[i][j] == 1){
                SDL_RenderCopy(
                        gRenderer,
                        gTextureLatticeCover,
                        nullptr,
                        return_lattice_rect(i, j));
            }
        }
    }
}

int32_t Interface::check_bound_xy(
        const int32_t cur_x,
        const int32_t cur_y,
        const int32_t add_x,
        const int32_t add_y,
        const int32_t upper) const {
    if (cur_x + add_x < 0 || cur_x + add_x > 4) {
        return 0;
    }

    if (cur_y + add_y < 0 || cur_y + add_y > 4) {
        return 0;
    }

    if (exist[upper][cur_x + add_x][cur_y + add_y] > 0) {
        return 2;
    }

    if (int32_t notUpper = (upper == 1) ? 0 : 1 ;
            exist[notUpper][cur_x + add_x][cur_y + add_y] > 0) {
        return 0;
    }

    return 1;
}

SDL_Rect* Interface::return_lattice_rect(
        const int32_t x,
        const int32_t y) {
    temp.x = 140 + 77 * x;
    temp.y = 140 + 77 * y;
    temp.w = 76;
    temp.h = 76;

    return &temp;
}

bool Interface::match_rect_xy(
        const int32_t& x,
        const int32_t& y,
        const SDL_Rect& rect) const {
    if ((x >= rect.x && x <= rect.x + rect.w)
            && (y >= rect.y && rect.y + rect.h)) {
        return true;
    }
    return false;
}

std::pair<int32_t, int32_t> Interface::return_MouseIndex(
        const int32_t x,
        const int32_t y) const {
    int32_t index_x = (x - 140) / 77;
    int32_t index_y = (y - 140) / 77;

    if (x - 140 < 0) {
        index_x = ((x - 140 - 77) / 77);
    }

    if (y - 140 < 0) {
        index_y = ((y - 140 - 77) / 77);
    }

    std::cout << "index: " << index_x;
    std::cout << ", index_y: " << index_y << std::endl;

    return std::make_pair(index_x, index_y);
}

void Interface::setup_bmp_size(
        SDL_Rect *dest,
        const int32_t x,
        const int32_t y,
        const int32_t w,
        const int32_t h) const {
    dest->x = x;
    dest->y = y;
    dest->w = w;
    dest->h = h;
}

bool Interface::loadMedia(
        SDL_Surface **gHelloWorld,
        char const *str) {
    // Loading success flag
    bool success = true;

    // Load splash image
    *gHelloWorld = loadSurface(str);

    if (*gHelloWorld == nullptr) {
        std::cerr << "Unable to load image " << str << std::endl;
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }

    return success;
}

bool Interface::loadMedia(
        SDL_Texture **gTexture,
        char const *str) {
    // Loading success flag
    bool success = true;

    // Load splash image
    *gTexture = loadTexture(str);

    if (*gTexture == nullptr) {
        std::cerr << "Unable to load image " << str << std::endl;
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }

    return success;
}

SDL_Surface* Interface::loadSurface(char const *str) {
    // The final optimized image
    SDL_Surface* optimizedSurface = nullptr;

    // Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << str << std::endl;
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    } else {
        // Convert surface to screen format
        optimizedSurface = SDL_ConvertSurface(
                loadedSurface,
                screenSurface->format,
                0);
        if (optimizedSurface == nullptr) {
            std::cerr << "Unable to optimize image " << str << std::endl;
            std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        }
        // Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }
    return optimizedSurface;
}

SDL_Texture* Interface::loadTexture(char const *str) {
    // The final texture
    SDL_Texture* newTexture = nullptr;

    // Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);

    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << str << std::endl;
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    } else {
        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cerr << "Unable to create texture from " << str << std::endl;
            std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        }
        // Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

void Interface::close() {
    // Free loaded image
    SDL_DestroyTexture(gTextureBoard);
    gTextureBoard = nullptr;

    // Deallocate surface
    SDL_FreeSurface(screenBoard);
    screenBoard = nullptr;

    // Destroy window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(window);
    window = nullptr;
    gRenderer = nullptr;

    // Quit SDL subsystems
    SDL_Quit();
}


bool Interface::isWithinBound (std::pair<int32_t, int32_t> TEMP) const {
    auto& [x, y] = TEMP;
    if (x < 0 || x > 4 || y < -2 || y > 6){
        return false;
    }
    return true;
}

bool Interface::isRepeat() {
    uint32_t c_key = 0;
    uint32_t c_store[4] = {0};

    int32_t count = 0;

    for (int32_t y = 0 ; y < ROW_SIZE_ ; y++) {
        for (int32_t x = 0 ; x < COL_SIZE_ ; x++) {
            uint8_t t_key = 0;
            for (int32_t i = 0 ; i < PLAYER_NUMBER_ ; i++) {
                if (int32_t kind = exist[i][x][y];
                        kind > 0) {
                    t_key |= i;

                    if (kind >= ROOKUP_) {
                        kind -= LEVEL_CHANGE_;
                        t_key |= (1 << 1);
                    }

                    t_key |= (1 << (kind - 1));
                }

            }
            if (t_key > 0) {
                auto& t_store = c_store[count / 4];
                t_store |= ( t_key << ((count & 3) << 3) );
                count += 1;

                c_key |= (1 << (x) << ((y << 2) + y));
                if (c_key & (31 << ((y << 2) + y + 5))) {
                    c_key |= (1 << y);
                }
            }
        }
    }

    bool check = true;
    for (int32_t j = 0 ; j < 5 ; j++) {
        if (c_key == key_[j]) {
            for (int32_t i = 0 ; i < 3 ; i++) {
                if (c_store[i] != store_[j][i]) {
                    check = false;
                    break;
                }
            }
        }else {
            check = false;
        }

        if (check == true) {
            break;
        }
    }

    if (check) {
        repeat_times_ += 1;
    } else {
        repeat_times_ = 1;
    }

    key_[repeat_index_ & 4] = c_key;
    std::cout << "key: " << key_[repeat_index_ & 4] << std::endl;
    std::cout << "store: ";
    for (int32_t i = 0 ; i < 4 ; i++) {
        store_[repeat_index_ & 4][i] = c_store[i];
        std::cout << store_[repeat_index_ & 4][i] << " ";
    }
    std::cout << std::endl;

    repeat_index_ += 1;

    return repeat_times_ >= 3;
}
