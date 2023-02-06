#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    char* code = getCode("test.code");

    progDump(Compile(code));
    return 0;
}
