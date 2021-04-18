/* Copyright 2021
 * 林育辰 Yu-Chen Lin
 * 國立臺灣師範大學 National Taiwan Normal University
 * 資訊工程學系 Computer Science and Information Engineering
 */

#include "interface.h"

#define PRINT

void Interface::InitMedia() {
    loadMedia(
            &gTextureBackground,
            const_cast<char *> ("../image/background.bmp"));
    loadMedia(
            &gTextureKing,
            const_cast<char *> ("../image/king.bmp"));
    loadMedia(
            &gTextureJade,
            const_cast<char *> ("../image/Jade.bmp"));
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
    memset(exist, 0, sizeof(exist));

    exist[0][0][0] = exist[1][4][4] = 2;
    exist[0][1][0] = exist[1][3][4] = 3;
    exist[0][2][0] = exist[1][2][4] = 5;
    exist[0][3][0] = exist[1][1][4] = 4;
    exist[0][4][0] = exist[1][0][4] = 1;
    exist[0][4][1] = exist[1][0][3] = 6;

    std::fill(walking.begin(), walking.end(), std::vector<int32_t>(N_, 0));
    mouseIndex_ = std::make_pair(-1, -1);
}

bool Interface::DetectKingExist() {
    int32_t exist_king = 0;
    for (int32_t i = 0 ; i < 2 ; i++) {
        for (int32_t j = 0 ; j < 5 ; j++) {
            for (int32_t k = 0 ; k < 5 ; k++) {
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
    setup_bmp_size(&texPosition_[0], 0, 0, SCREEN_WIDTH * 1.2, SCREEN_HEIGHT);
    setup_bmp_size(&texPosition_[1], 140, 140, 380, 380);

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

    for (int32_t i = 0 ; i <= 5 ; i++) {
        for (int32_t j = 5 ; j <= 6 ; j++) {
            setup_bmp_size(
                    &chessDect_[i][j],
                    155 + 77 * i,
                    152 - 75 * (j-4),
                    125,
                    250);
        }
    }

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
            "Mini-shogi",                   // Title
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
    static int32_t push_stat[2][2] = {
        {0, 5},
        {0, 5}
    };

    if (chess >= 7 && chess <= 12) {
        chess -= 5;
    }

    for (int32_t i = 0 ; i < 2 ; i++) {
        if (push_stat[i][0] >= 5) {
            push_stat[i][0] = 0;
            push_stat[i][1] += 1;
        }
    }

    exist[kind][push_stat[kind][0]][push_stat[kind][1]] = chess;
    push_stat[kind][0] += 1;
}

void Interface::MoveChess(
        std::pair<int32_t, int32_t> ori,
        std::pair<int32_t, int32_t> des) {
    int32_t kind = exist[0][ori.first][ori.second] > 0 ? 0 : 1;
    int32_t kind_des = exist[0][des.first][des.second] > 0 ? 0 : 1;
    int32_t IsChess = 0;
    int32_t walk_check = walking[des.first][des.second];

#ifdef PRINT
    std::cout << "ori: " << ori.first << " " << ori.second << std::endl;
    std::cout << "des: " << des.first << " " << des.second << std::endl;
    std::cout << "kind: " << kind << ", " << kind_des << std::endl;
    std::cout << "walk_check: " << walking[des.first][des.second] << std::endl;
#endif

    if (kind == 1 && exist[1][ori.first][ori.second] == 0) {
        return;
    }

    if (walk_check == 0) {
        return;
    }

    if (exist[kind == 1 ? 0 : 1][des.first][des.second] > 1) {
        IsChess = 1;
    }

    if (kind_des != 1 && kind == 0) {
        return;
    }

    if (IsChess != 0) {
        auto& [fir, sec] = des;
        CaptivePush(kind, exist[kind == 1 ? 0 : 1][fir][sec]);
    }

    exist[kind][des.first][des.second]=\
                                       exist[kind][ori.first][ori.second];
    exist[kind][ori.first][ori.second] = 0;
    exist[kind == 1 ? 0 : 1][des.first][des.second] = 0;

    if ((ori.second == 3 * (kind == 1 ? 0 : 1)
                || ori.second == 1 + 3 * (kind == 1 ? 0 : 1))
            && (des.second == 3 * (kind == 1 ? 0 : 1)
                || des.second == 1 + 3 * (kind == 1 ? 0 : 1))) {
        int32_t chess = exist[kind][des.first][des.second];
        if ((chess >= 2 && chess <= 6)  && chess != 4) {
            exist[kind][des.first][des.second] += 5;
        }
    }
}

void Interface::PrintBugMessageBoard() const {
    system("clear");
    for (int32_t k = 0 ; k < 2 ; k++) {
        std::cout << "------------" << std::endl;
        for (int32_t i = 0 ; i < 7 ; i ++) {
            for (int32_t j = 0 ; j < 5 ; j ++) {
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
    if (fMouseIndex.first >= 0
            && fMouseIndex.first <= 4
            && fMouseIndex.second >= 0
            && fMouseIndex.second <= 4) {
        if (exist[1][fMouseIndex.first][fMouseIndex.second] > 0) {
            SDL_RenderCopy(
                    gRenderer,
                    gTextureAlphaChess,
                    nullptr,
                    &chessDect_[fMouseIndex.first][fMouseIndex.second]);
        } else if (exist[0][fMouseIndex.first][fMouseIndex.second] > 0) {
            SDL_RenderCopyEx(
                    gRenderer,
                    gTextureAlphaChess,
                    nullptr,
                    &chessDect_[fMouseIndex.first][fMouseIndex.second],
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

void Interface::make_walking(const std::pair<int32_t, int32_t> temp) {
    std::fill(walking.begin(), walking.end(), std::vector<int32_t>(N_, 0));

    int32_t Isupper = 0, kind = 0;
    auto &[fir, sec] = temp;

    if (exist[0][fir][sec] > 0) {
        Isupper = 1;
        kind = exist[0][fir][sec];
    } else if (exist[1][fir][sec] > 0) {
        Isupper = 0;
        kind = exist[1][fir][sec];
    } else {
        return;
    }

    int32_t direction = Isupper ? 1 : -1;

    if (check_bound_xy(fir, sec, -1, direction, Isupper)) {
        if (kind == KING_
                || kind == GOLD_
                || kind == SLIVER_
                || kind == ROOKUP_
                || kind == BISHOPUP_
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir-1][sec + direction] = 1;
        }
    }

    if (check_bound_xy(fir, sec, 0, direction, Isupper)) {
        if (kind == KING_
                || (kind >= GOLD_ && kind <= BISHOPUP_)
                || kind == SLIVERUP_
                || kind == PAWNUP_) {
            walking[fir][sec + direction] = 1;
        }
    }

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

    if (check_bound_xy(fir, sec, 1, 0, Isupper)) {
        if (kind == 1
                || kind == 4
                || kind == 7
                || kind == 8
                || kind == 10
                || kind == 11) {
            walking[fir + 1][sec] = 1;
        }
    }

    if (check_bound_xy(fir, sec, -1, -1 * direction, Isupper)) {
        if (kind == 1 || kind == 5 || kind == 7 || kind == 8) {
            walking[fir - 1][sec - direction] = 1;
        }
    }

    if (check_bound_xy(fir, sec, 0, -1 * direction, Isupper)) {
        if (kind == 1
                || kind == 4
                || kind == 7
                || kind == 8
                || kind == 10
                || kind == 11) {
            walking[fir][sec - direction] = 1;
        }
    }

    if (check_bound_xy(fir, sec, 1, -direction, Isupper)) {
        if (kind == 1 || kind == 5 || kind == 7 || kind == 8) {
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

    if (kind != 2 && kind != 3 && kind - 5 != 2 && kind - 5 != 3) {
        return;
    }


    for (int32_t i = (kind == 3 || kind == 8) ? 0 : 1 ; i < 8 ; i += 2) {
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
    make_walking(temp);
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

    if (int32_t notUpper = (upper == 1) ? 0 : 1 ; exist[notUpper][cur_x + add_x][cur_y + add_y] > 0) {
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

void Interface::run(){
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

                while (SDL_PollEvent(&e)) {
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
