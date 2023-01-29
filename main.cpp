#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    FILE* file = fopen("code.txt", "r");
    char* code = (char*)calloc(10000, sizeof(char));
    strcpy(code, "$boba = aboba(1,2); _(){} ^aboba($a, $b){}");
   
    progDump(Compile(code));
    return 0;
}
