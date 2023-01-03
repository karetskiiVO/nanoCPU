#ifndef sLANG
#define sLANG

#include "clist_str.h"
#include "spectypes.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct block_t {
    char* name;
    List* varlist;
    bool  isfunc;
    bool  isCall;
    bool  isVoid;
} block_t;

typedef struct prog_t {
    CODE_TYPE type;
    ///content///

    double   value;
    COP_TYPE nodeop;
    char*    varname;
    block_t* block;

    prog_t* left;
    prog_t* right;
} prog_t;





#endif