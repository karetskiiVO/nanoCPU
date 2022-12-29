#include "$lang.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define checklex(_lex) (!strncmp(#_lex, *ptr, strlen(#_lex)) && (*ptr += strlen(#_lex)))

static void skipSpaces (char* *ptr) {
    while (strchr(" \t\r\n", **ptr)) (*ptr)++;
}

static prog_t* getProgramm (char* code) {
    
}

static prog_t* getVar (char* *ptr, List* varlist) {
    char* var = startVar(ptr, varlist);
    if (!var) return NULL;
    skipSpaces(ptr);
    
    if (checklex(=)) {
        // getEq
    }

    if (checklex(;)) {

    } else {
        // error
        return NULL;
    }
}

static char* startVar (char* *ptr, List* varlist) {
    skipSpaces(ptr);
    if (!checklex($)) return NULL;
    
    skipSpaces(ptr);
    size_t len = 0, varpos = 0;
    char   varname[1000] = ""; //rewrite this sh!t
    sscanf(*ptr, "%[^ =\t\r\n;]%ln", varname, &len);
    *ptr += len;

    if (listSearch(varlist, varname) != EMPTY) {
        // error - double definition
    }

    Elem_t newvar = {0};
    newvar.name   = (char*)calloc(strlen(varname) + 1, sizeof(char));
    strcpy(newvar.name, varname);
    varpos = listAdd(varlist, 0, newvar);

    return varlist->arr[varpos].value.name;
}


/** TODO
 * 0. newNode....
 * 1. getEq
 * 2. precompile
 * 3. stack of lists
 * 4. list of functions
 * 
 */




