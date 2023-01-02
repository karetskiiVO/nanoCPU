#include "$lang.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cstack_var.h"

#define checklex(_lex) (!strncmp(#_lex, *ptr, strlen(#_lex)) && (*ptr += strlen(#_lex)))
#define Checklex(_lex) (!strncmp(_lex, *ptr, strlen(_lex)) && (*ptr += strlen(_lex)))

static void progDump_node (prog_t* node, FILE* dmp);
static void progDump_edge (prog_t* node, FILE* dmp);

void progDump (prog_t* prog, const char* filename) {
    FILE* dmp = fopen(filename, "w");

    fprintf(dmp, "digraph g {\n\t{\n");
    progDump_node(prog, dmp);

}

static void progDump_node (prog_t* node, FILE* dmp) {

}

static void progDump_edge (prog_t* node, FILE* dmp) {

} 

static void skipSpaces (char* *ptr) {
    while (strchr(" \t\r\n", **ptr)) (*ptr)++;
}



static prog_t* newProgNodeOp (COP_TYPE optype, prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->nodeop = optype;
    ret->left   = left;
    ret->right  = right;

    return ret;
}

static prog_t* newProgNodeOpCst (double val) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->value  = val;
    return ret;
}

static prog_t* newProgNodeOpVar (const char* varname) {
    prog_t* ret  = (prog_t*)calloc(1, sizeof(prog_t));
    ret->varname = (char*)calloc(1 + strlen(varname), sizeof(char));
    strcpy(ret->varname, varname);

    return ret;
}



static prog_t* getProgramm (char* code) {
    
}

static prog_t* getVar (char* *ptr, Stack* vartable, List* funclist) {
    char* var = startVar(ptr, vartable, funclist);
    if (!var) return NULL;
    skipSpaces(ptr);

    prog_t* ret = NULL;

    if (checklex(=)) {
        ret = newProgNodeOp(COP_assign, newProgNodeOpVar(var), getEq(ptr, vartable, funclist));
    } else {
        ret = newProgNodeOp(COP_assign, newProgNodeOpVar(var), newProgNodeOpCst(0));
    }

    skipSpaces(ptr);
    if (checklex(;)) {
        return ret;
    } else {
        // error
        return NULL;
    }
}

static char* startVar (char* *ptr, Stack* vartable, List* funclist) {
    skipSpaces(ptr);
    if (!checklex($)) return NULL;
    
    skipSpaces(ptr);
    size_t len = 0, varpos = 0;
    char   varname[1000] = ""; //rewrite this sh!t
    sscanf(*ptr, "%[^ =\t\r\n;]%ln", varname, &len);
    *ptr += len;

    if (listSearch((List*)vartable->arr[vartable->size - 1], varname) != EMPTY) {
        // error
    }

    List* varlist = (List*)vartable->arr[vartable->size - 1];

    Elem_t newvar = {0};
    newvar.name   = (char*)calloc(strlen(varname) + 1, sizeof(char));
    strcpy(newvar.name, varname);
    varpos = listAdd(varlist, 0, newvar);

    return varlist->arr[varpos].value.name;
}

static prog_t* getEq  (char* *ptr, Stack* vartable, List* funclist) {
    return getSum(ptr, vartable, funclist);
}

static prog_t* getSum (char* *ptr, Stack* vartable, List* funclist) {
    prog_t* val = getMul(ptr, vartable, funclist);
    prog_t* buf = val;

    skipSpaces(ptr);
    while (**ptr == '+' || **ptr == '-') {
        char com = **ptr;
        (*ptr)++;

        if (com == '+') {
            val = newProgNodeOp(COP_add, buf, getMul(ptr, vartable, funclist));
        } else {
            val = newProgNodeOp(COP_sub, buf, getMul(ptr, vartable, funclist));
        }

        buf = val;
    }

    return val;
}

static prog_t* getMul (char* *ptr, Stack* vartable, List* funclist) {
    prog_t* val = getPow(ptr, vartable, funclist);
    prog_t* buf = val;

    skipSpaces(ptr);
    while (**ptr == '*' || **ptr == '/') {
        char com = **ptr;
        (*ptr)++;

        if (com == '*') {
            val = newProgNodeOp(COP_mul, buf, getPow(ptr, vartable, funclist));
        } else {
            val = newProgNodeOp(COP_div, buf, getPow(ptr, vartable, funclist));
        }

        buf = val;
    }

    return val;
}

static prog_t* getPow (char* *ptr, Stack* vartable, List* funclist) {
    prog_t* val = getFnc(ptr, vartable, funclist);
    prog_t* buf = val;

    skipSpaces(ptr);
    while (**ptr == '^') {
        char com = **ptr;
        (*ptr)++;
        val = newProgNodeOp(COP_pow, buf, getFnc(ptr, vartable, funclist));

        buf = val;
    }

    return val;
}

static prog_t* getFnc (char* *ptr, Stack* vartable, List* funclist) {
    char func[1000] = "";
    size_t  len = 0;
    prog_t* ret = NULL;

    skipSpaces(ptr);
    sscanf(*ptr, "%[^()]%ln", func, &len);
    if (len == 0) {
        Checklex("(");
        ret = getEq(ptr, vartable, funclist);
        Checklex(")");
    } else {
        *ptr += len;
        size_t funcind = listSearch(funclist, func);
        if (funcind == EMPTY) {
            // error
        }
        if (!Checklex("(")) {
            // error
        }
        
        /////////////////////////////////////////
        prog_t* buf = ret;

        skipSpaces(ptr);
        if (funclist->arr[funcind].value.val > 0) {
            
        }

        for (int i = 0; i < funclist->arr[funcind].value.val; i++) {
            skipSpaces(ptr);
            if (!Checklex(",")) {
                // error
            }


        }

        if (!Checklex(")")) {
            // error
        }
    }

    return ret;
}


/** TODO
 * 0. newNode....
 * 1. getEq
 * 2. precompile
 * 3. stack of lists
 * 4. list of funclisttions
 * 
 */




