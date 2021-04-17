// ////////////////////////////////////////////////////////////////
// >>> project.c <<<
//FUNCION IMPLEMENTATION
//
#include "project.h"


///////////////////////////////////
//MEAU
//
//  >>> Main meau <<<
//
// >>> Key reminder <<<
void reminder(){
    system("clear");

    fprintf(stderr, "[---------REMIDER---------]\n");
    fprintf(stderr, "* " GRN "[Backspace]" reset " back page.\n");
    fprintf(stderr, "* " BLU "[Esc]" reset " terminate the program.\n"); 
    fprintf(stderr, "* [Other] Real-time detection when you choose.\n");
    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, "Press ANY key to start...");

    getch();
    //usleep(500000);
}

///////////////////////////////////
//I/O
//
//
//
//USING THE HOMEWORK SIX 
// >>> getch POSFIX <<< 
int32_t getch(){
    int32_t ch;

    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    memcpy(&newt, &oldt, sizeof(newt));

    newt.c_lflag &= ~( ECHO | ICANON | ECHOE | ECHOK |
            ECHONL | ECHOPRT | ECHOKE | ICRNL);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

bool read_str(char str[]){
    int32_t index = 0;

    char tmp;
    while ((tmp = getch()) != 10){
        if (tmp == EOF)
            return 1;

        if (tmp == 127 && index){
            printf("\b \b");
            index -= 1;
        }else if (tmp != 127){
            printf("%c", tmp);
            str[index++] = tmp;
        }
    }
    str[index] = '\0';
    printf("\n");

    return 0;
}
//
// >>> project.c <<<
// END
//////////////////////////////////////////////////////////////////
