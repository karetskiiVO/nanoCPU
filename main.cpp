#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    char* code = (char*)calloc(10000, sizeof(char));
    strcpy(code, " $s1 = 0; _(){$s2 = 4; {$s2=2;}}");
   
    progDump(Compile(code));
    return 0;
}
