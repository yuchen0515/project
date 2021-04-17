// ////////////////////////////////////////////////////////////////
// >>> project.h <<<
//FUNCION PROTOTYPE
//
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// POSIX headers
#include <termios.h>  // for tcxxxattr, ECHO, etc ..
#include <unistd.h>    // for STDIN_FILENO

#define GRN "\e[0;32m"
#define BLU "\e[0;34m"
#define reset "\e[0m"

void reminder();
int32_t getch();
bool read_str(char str[]);

//
// >>> project.h <<<
// END
//////////////////////////////////////////////////////////////////
