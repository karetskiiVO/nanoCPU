#include "Slang.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main () {
    char* code = getCode("test.code");

    vector<block_t> prog = Compile(code);
    progDump(prog);
    return 0;
}
