#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    FILE* file = fopen("code.txt", "r");
    char* code = (char*)calloc(10000, sizeof(char));
    strcpy(code, "$sus;\n $a;$b;$soda;");
    progDump(getProgram(&code));
    return 0;
}
