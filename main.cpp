#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {

    FILE* file = fopen("test.code", "r");
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* code = (char*)calloc(fsize + 1, sizeof(char)); 
    fread(code, fsize, 1, file);

    fclose(file);

    progDump(Compile(code));
    return 0;
}
