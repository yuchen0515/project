#include "interface.h"

using namespace std;

//Screen dimension constants  螢幕寬高設定
const int32_t SCREEN_WIDTH = 640;
const int32_t SCREEN_HEIGHT = 640;

SDL_Surface *screenBoard= NULL;
SDL_Surface *screenSurface = NULL;
SDL_Window *window = NULL;
SDL_Rect temp;

//current dusokated textyre
SDL_Texture *gTextureBoard = NULL;
SDL_Texture *gTextureBackground= NULL;
SDL_Texture *gTextureKing= NULL;
SDL_Texture *gTextureJade= NULL;
SDL_Texture *gTextureGold= NULL;
SDL_Texture *gTextureSliver= NULL;
SDL_Texture *gTextureSliverUp= NULL;
SDL_Texture *gTextureBishop= NULL;
SDL_Texture *gTextureBishopUp= NULL;
SDL_Texture *gTexturePawn= NULL;
SDL_Texture *gTexturePawnUp= NULL;
SDL_Texture *gTextureRook= NULL;
SDL_Texture *gTextureRookUp= NULL;
SDL_Texture *gTextureAlphaChess= NULL;
SDL_Texture *gTextureLatticeCover= NULL;
SDL_Texture *gTextureShow= NULL;

//0: upper, 1: lower
//
//1: king, 2: rook, 3: bishop, 4: gold
//5: sliver, 6: pawn
//7: rook (up), 8: bishop (up), 9: reserve
//10: sliver (up), 11: pawn(up)
//12 ~ 17: captive 1~6
//
int32_t exist[2][5][7];
int32_t walking[5][5];

//The window renderer
SDL_Renderer *gRenderer = NULL;

pair<int32_t, int32_t> MouseIndex = make_pair(-1, -1);
pair<int32_t, int32_t> MouseIndexTemp = make_pair(-1, -1);
int32_t MouseX = 0, MouseY = 0;
bool bClickChess = false;

SDL_Rect NoMove[2];
SDL_Rect ChessDect[5][5];
SDL_Point ChessSize = (SDL_Point){25, 30};

void InitMedia(){
    loadMedia(&gTextureBackground, (char *)"background.bmp");
    loadMedia(&gTextureKing   , (char *)"king.bmp");
    loadMedia(&gTextureJade   , (char *)"Jade.bmp"); 
    loadMedia(&gTextureGold   , (char *)"gold.bmp"); 
    loadMedia(&gTextureSliver , (char *)"sliver.bmp");   
    loadMedia(&gTextureSliverUp , (char *)"sliverup.bmp");   
    loadMedia(&gTextureBishop , (char *)"bishop.bmp");   
    loadMedia(&gTextureBishopUp , (char *)"bishopup.bmp");   
    loadMedia(&gTexturePawn   , (char *)"pawn.bmp"); 
    loadMedia(&gTexturePawnUp   , (char *)"pawnup.bmp"); 
    loadMedia(&gTextureRook   , (char *)"rook.bmp"); 
    loadMedia(&gTextureRookUp   , (char *)"rookup.bmp"); 
    loadMedia(&gTextureAlphaChess, (char *)"alphachess.bmp"); 
    loadMedia(&gTextureLatticeCover, (char *)"latticecover.bmp"); 
}

void InitExist(){
    memset(exist, 0, sizeof(exist));

    exist[0][0][0] = 2;
    exist[0][1][0] = 3;
    exist[0][2][0] = 5;
    exist[0][3][0] = 4;
    exist[0][4][0] = 1;
    exist[0][4][1] = 6;

    exist[1][4][4] = 2;
    exist[1][3][4] = 3;
    exist[1][2][4] = 5;
    exist[1][1][4] = 4;
    exist[1][0][4] = 1;
    exist[1][0][3] = 6;

    memset(walking, 0, sizeof(walking));
}

void InitPosition(){
    setup_bmp_size(&NoMove[0], 0, 0, SCREEN_WIDTH*1.2, SCREEN_HEIGHT);
    setup_bmp_size(&NoMove[1], 140, 140, 380, 380);

    for (int32_t i = 0 ; i < 5 ; i++){
        for (int32_t j = 0 ; j < 5 ; j++){
            setup_bmp_size(&ChessDect[i][j], 155 + 77 * i, 152 + 75 * j, 125, 250);
        }
    }
}

bool init(){

    InitExist();
    InitPosition();

    //

    //The window we'll be rendering to
    //The surface contained by the window
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL couldn't initialize!");
        printf("SDL_Error detail: %s\n", SDL_GetError());
        return 0;
    }

    //Set texture filtering to linear
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
        printf( "Warning: Linear texture filtering not enabled!" );
    }


    //Create window
    window = \
             SDL_CreateWindow( "Mini-shogi",     //window title 
                     SDL_WINDOWPOS_CENTERED,    //position setting X
                     SDL_WINDOWPOS_CENTERED,    //position setting Y
                     SCREEN_WIDTH,              //window width
                     SCREEN_HEIGHT,             //window height
                     SDL_WINDOW_SHOWN           //show window
                     );

    if (window == NULL){
        printf("Window couldn't be created!\n");
        printf("SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    //Create renderer for window
    gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if( gRenderer == NULL )
    {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }

    //Initialize renderer color
    SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 255 );

    return 1;
}


void CaptivePush(int32_t kind, int32_t chess){
    static int32_t push_stat[2][2] = 
    {
        {0, 5},
        {0, 5}
    };

    if (chess >= 7 && chess <= 12)
        chess -= 5;

    for (int32_t i = 0 ; i < 2 ; i++){
        if (push_stat[i][0] >= 5){
            push_stat[i][0] = 0;
            push_stat[i][1] += 1;
        }
    }

    exist[kind][push_stat[kind][0]++][push_stat[kind][1]] = chess;
}

void MoveChess(pair<int32_t, int32_t> ori, pair<int32_t, int32_t> des){
    int32_t kind = exist[0][ori.first][ori.second] > 0 ? 0 : 1;
    int32_t kind_des = exist[0][des.first][des.second] > 0 ? 0 : 1;
    int32_t IsChess = 0;
    int32_t walk_check = walking[des.first][des.second];

    printf("ori: %d %d\n", ori.first, ori.second);
    printf("des: %d %d\n", des.first, des.second);
    printf("kind: %d, kind_des: %d\n", kind, kind_des);
    printf("walk_check: %d\n", walking[des.first][des.second]);

    if (kind == 1 && exist[1][ori.first][ori.second] == 0)
        return;

    if (walk_check == 0)
        return;

    if (kind_des == 1){
        if (exist[kind == 1 ? 0 : 1][des.first][des.second] > 1)
            IsChess = 1;
    }
    else if (kind == 0)
        return ;

    printf("check\n");

    if (IsChess){
        CaptivePush(kind, exist[kind][des.first][des.second]);
    }

    exist[kind][des.first][des.second]=\
                exist[kind][ori.first][ori.second];
    exist[kind][ori.first][ori.second] = 0;
    exist[kind == 1 ? 0 : 1][des.first][des.second] = 0;

}

void PrintBugMessageBoard(){
    for (int32_t k = 0 ; k < 2 ; k++){
        printf("------------\n");
        for (int32_t i = 0 ; i < 5 ; i ++){
            for (int32_t j = 0 ; j < 5 ; j ++){
                printf("%d ", exist[k][j][i]);

            }
            printf("\n");
        }
        printf("------------\n");
    }

}

void Determine_Draw(int32_t kind, int32_t Isupper, int32_t j, int32_t k){
    switch (kind){
        case 1:
            if (Isupper)
                gTextureShow = gTextureKing;
            else if (!Isupper)
                gTextureShow = gTextureJade;
            break;
        case 2:
            gTextureShow = gTextureRook;
            break;
        case 3:
            gTextureShow = gTextureBishop;
            break;
        case 4:
            gTextureShow = gTextureGold;
            break;
        case 5:
            gTextureShow = gTextureSliver;
            break;
        case 6:
            gTextureShow = gTexturePawn;
            break;

        default:
            gTextureShow = NULL;

    }

    if (gTextureShow != NULL){
        if (Isupper == 1)
            SDL_RenderCopyEx(gRenderer, gTextureShow, NULL, &ChessDect[j][k], 180, &ChessSize, SDL_FLIP_NONE);
        else if (Isupper == 0)
            SDL_RenderCopy(gRenderer, gTextureShow, NULL, &ChessDect[j][k]);
    }

}

bool ClickCover(pair<int32_t, int32_t> fMouseIndex){
    if (fMouseIndex.first >= 0 && fMouseIndex.first <= 4 && fMouseIndex.second >= 0 && fMouseIndex.second <= 4){

        if (exist[1][fMouseIndex.first][fMouseIndex.second] > 0)
            SDL_RenderCopy(gRenderer, gTextureAlphaChess, NULL, &ChessDect[fMouseIndex.first][fMouseIndex.second]); 

        else if (exist[0][fMouseIndex.first][fMouseIndex.second] > 0)
            SDL_RenderCopyEx(gRenderer, gTextureAlphaChess, NULL, &ChessDect[fMouseIndex.first][fMouseIndex.second], 180, &ChessSize, SDL_FLIP_NONE); 

        return 1;
    }

    return 0;
}

void Show_Chess(){
    for (int32_t i = 0 ; i < 2 ; i++){
        for (int32_t j = 0 ; j < 5 ; j++){
            for (int32_t k = 0 ; k < 5 ; k++){
                Determine_Draw(exist[i][j][k], (1-i), j, k);
            }
        }
    }
}

void show_walking(pair<int32_t, int32_t> temp){

    memset(walking, 0, sizeof(walking));

    int32_t Isupper = 0, kind = 0;
    int32_t cur_x = temp.first;
    int32_t cur_y = temp.second;

    if (exist[0][temp.first][temp.second] > 0){
        Isupper = 1;
        kind = exist[0][temp.first][temp.second];
    }else if (exist[1][temp.first][temp.second] > 0){
        Isupper = 0;
        kind = exist[1][temp.first][temp.second];
    }else
        return;

    int32_t direction = Isupper ? 1 : -1;

    if (check_bound_xy(temp.first, temp.second, -1, direction, Isupper)){
        if (kind == 1 || kind == 4 || kind == 5){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first - 1, temp.second + direction ));
            walking[temp.first-1][temp.second + direction] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, 0, direction, Isupper)){
        if (kind == 1 || kind == 4 || kind == 5 || kind == 6){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first, temp.second + direction ));
            walking[temp.first][temp.second + direction] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, 1, direction, Isupper)){
        if (kind == 1 || kind == 4 || kind == 5){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first + 1, temp.second + direction ));
            walking[temp.first + 1][temp.second + direction] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, -1, 0, Isupper)){
        if (kind == 1 || kind == 4){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first - 1, temp.second));
            walking[temp.first - 1][temp.second] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, 1, 0, Isupper)){
        if (kind == 1 || kind == 4){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first + 1, temp.second));
            walking[temp.first + 1][temp.second] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, -1, -1 * direction, Isupper)){
        if (kind == 1 || kind == 5){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first - 1, temp.second - direction));
            walking[temp.first - 1][temp.second - direction] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, 0, -1 * direction, Isupper)){
        if (kind == 1 || kind == 4){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first, temp.second - direction));
            walking[temp.first][temp.second - direction] = 1;
        }
    }

    if (check_bound_xy(temp.first, temp.second, 1, direction, Isupper)){
        if (kind == 1 || kind == 5){
            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first + 1, temp.second + direction));
            walking[temp.first + 1][temp.second + direction] = 1;
        }
    }

    int32_t dec[8][2] =
    {
        {-1 , direction},
        {0, direction},
        {1, direction},
        {1, 0},
        {1, -1 * direction},
        {0, -1 * direction},
        {-1, -1 * direction},
        {-1, 0}
    };

    if (kind != 2 && kind != 3)
        return;


    for (int32_t i = (kind == 3) ? 0 : 1 ; i < 8 ; i += 2){
        int32_t level = 1;
        while (1){
            if (check_bound_xy(temp.first, temp.second, dec[i][0] * level, dec[i][1] * level, Isupper) == 0)
                break;

            SDL_RenderCopy(gRenderer, gTextureLatticeCover, NULL, return_lattice_rect(temp.first + dec[i][0] * level, temp.second + dec[i][1] * level));
            walking[temp.first + dec[i][0] * level][temp.second + dec[i][1] * level] = 1;

            if (check_bound_xy(temp.first, temp.second, dec[i][0] * level, dec[i][1] * level, Isupper) == 2)
                break;

            level += 1;

        }
    }
}

int32_t check_bound_xy(int32_t cur_x, int32_t cur_y, int32_t add_x, int32_t add_y, int32_t upper){
    if (cur_x + add_x < 0 || cur_x + add_x > 4)
        return 0;

    if (cur_y + add_y < 0 || cur_y + add_y > 4)
        return 0;


    if (exist[upper][cur_x + add_x][cur_y + add_y] > 0)
        return 2;

    upper = upper == 1 ? 0 : 1;

    if (exist[upper][cur_x + add_x][cur_y + add_y] > 0)
        return 0;

    return 1;
}


SDL_Rect *return_lattice_rect(int32_t x, int32_t y){
    temp.x = 140 + 77 * x;
    temp.y = 140 + 77 * y;
    temp.w = 76;
    temp.h = 76;

    return &temp;
}

bool match_rect_xy(int32_t x, int32_t y, SDL_Rect rect){
    if ((x >= rect.x && x <= rect.x + rect.w) && (y >= rect.y && rect.y + rect.h))
        return 1;
    return 0;
}

pair<int32_t, int32_t> return_MouseIndex(int32_t x, int32_t y){
    int32_t index_x = (x - 140) / 77;
    int32_t index_y = (y - 140) / 77;

    return make_pair(index_x, index_y);
}

void setup_bmp_size(SDL_Rect *dest, int32_t x, int32_t y, int32_t w, int32_t h){
    dest->x = x;
    dest->y = y;
    dest->w = w;
    dest->h = h;
}

bool loadMedia(SDL_Surface **gHelloWorld, char *str){
    //Loading success flag
    bool success = true;

    //Load splash image
    *gHelloWorld = loadSurface(str);

    if (*gHelloWorld == NULL )
    {
        printf("Unable to load image %s!\n", str);
        printf("SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

bool loadMedia(SDL_Texture **gTexture, char *str){
    //Loading success flag
    bool success = true;

    //Load splash image
    *gTexture= loadTexture(str);

    if (*gTexture == NULL )
    {
        printf("Unable to load image %s!\n", str);
        printf("SDL Error: %s\n", SDL_GetError());
        success = false;
    }

    return success;
}

SDL_Surface* loadSurface( char *str ){
    //The final optimized image
    SDL_Surface* optimizedSurface = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", str, SDL_GetError() );
    }
    else
    {
        //Convert surface to screen format
        optimizedSurface = SDL_ConvertSurface(loadedSurface, screenSurface->format, 0 );
        if( optimizedSurface == NULL )
        {
            printf( "Unable to optimize image %s! SDL Error: %s\n", str, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return optimizedSurface;
}

SDL_Texture *loadTexture(char *str){
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);

    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", str, SDL_GetError());
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", str, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

void close(){
    //Free loaded image
    SDL_DestroyTexture( gTextureBoard );
    gTextureBoard = NULL;

    //Deallocate surface
    SDL_FreeSurface(screenBoard);
    screenBoard = NULL;

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow(window);
    window = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}
