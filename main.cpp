#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    FILE* file = fopen("code.txt", "r");
    char* code = (char*)calloc(10000, sizeof(char));
    strcpy(code, "$aboba = 2; $b = 1 + aboba;");
    /*Work:
        $aboba;
        $boba = aboba;
      Don't work
        $aboba = 2;
        $boba = aboba;
    */
    //getProgram(&code);
    progDump(getProgram(&code));
    return 0;
}
