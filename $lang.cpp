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

static size_t skipSpaces (char* *ptr) {
    size_t empty = 0;
    while (strchr(" \t\r\n", **ptr)) {
        (*ptr)++;
        empty++;
    }

    return empty;
}



static prog_t* newProgNodeOp (COP_TYPE optype, prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->nodeop = optype;
    ret->type   = CODE_op;
    ret->left   = left;
    ret->right  = right;

    return ret;
}

static prog_t* newProgNodeOpCst (double val) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->value  = val;
    ret->type   = CODE_op;

    return ret;
}

static prog_t* newProgNodeOpVar (const char* varname) {
    prog_t* ret  = (prog_t*)calloc(1, sizeof(prog_t));
    ret->varname = (char*)calloc(1 + strlen(varname), sizeof(char));
    ret->type    = CODE_op;
    strcpy(ret->varname, varname);

    return ret;
}

static prog_t* newProgNodeEmpty (prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->type  = CODE_e;
    ret->left  = left;
    ret->right = right;

    return ret;
}

static prog_t* newProgNodeCall  (const char* name) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->type = CODE_block;
    ret->block->isfunc = true;
    ret->block->isCall = true;
    ret->block->name = (char*)calloc(1 + strlen(name), sizeof(char));
    strcpy(ret->block->name, name);

    return ret;
}



static prog_t* getProgramm (char* code) {
    char*  *ptr = &code;
    prog_t* ret = NULL;

    List* var_glob = NULL;
    List* funclist = NULL;
    listCtor(var_glob, 1);
    listCtor(funclist,  1);

    Stack* vartable = NULL;
    stackCtor_(vartable, 1);
    stackPush_(vartable, var_glob);

    bool isReadble = true;
    while (isReadble) {
        prog_t* buf_elem = getVar(ptr, vartable, funclist);
        if (!buf_elem) {
            buf_elem = getFunc(ptr, vartable, funclist);

            if (!buf_elem) break;
        } 

        prog_t* ret = newProgNodeEmpty(buf_elem, ret); // rewrite this sh!t
    }

    return ret;
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
    char   varname[1000] = ""; // rewrite this sh!t
    sscanf(*ptr, "%[^ =-+*/^&!@#%(){}[]?<>,.~`'\"\\|\t\r\n;]%ln", varname, &len);
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
    prog_t* val = getCll(ptr, vartable, funclist);
    prog_t* buf = val;

    skipSpaces(ptr);
    while (**ptr == '^') {
        char com = **ptr;
        (*ptr)++;
        val = newProgNodeOp(COP_pow, buf, getCll(ptr, vartable, funclist));

        buf = val;
    }

    return val;
}

static prog_t* getCll (char* *ptr, Stack* vartable, List* funclist) {
    char func[1000] = "";
    size_t  len = 0;
    prog_t* ret = NULL;

    skipSpaces(ptr);
    sscanf(*ptr, "%[^ =-+*/^&!@#%(){}[]?<>,.~`'\"\\|\t\r\n;]%ln", func, &len);

    if (!strlen(func)) {
        Checklex("(");
        ret = getEq(ptr, vartable, funclist);
        Checklex(")");
    } else {
        *ptr += len;
        size_t len1 = skipSpaces(ptr);

        if (!Checklex("(")) {
            *ptr -= (len + len1);

            return getNum(ptr, vartable, funclist);
        } else {
            size_t funcind = listSearch(funclist, func);
            if (funcind == EMPTY) {
                // error
            }

            // TODO: check non-void

            ret = newProgNodeCall(func);
            prog_t* buf = NULL;

            skipSpaces(ptr);
            if (funclist->arr[funcind].value.val > 0) {
                buf = newProgNodeEmpty(getEq(ptr, vartable, funclist), buf);        
            }

            for (int i = 0; i < funclist->arr[funcind].value.val; i++) {
                skipSpaces(ptr);
                if (!Checklex(",")) {
                    // error
                }

                buf = newProgNodeEmpty(getEq(ptr, vartable, funclist), buf);
            }

            ret->right = buf;
            if (!Checklex(")")) {
                // error
            }
        }
    }

    return ret;
}

static prog_t* getNum (char* *ptr, Stack* vartable, List* funclist) {
    size_t len = 0;
    double val = 0;

    if (sscanf(*ptr, "%lg%ln", &val, &len)) {
        *ptr += len;

        return newProgNodeOpCst(val);
    } 

    char varname[1000] = ""; // rewrite this sh!t
    sscanf(*ptr, "%[^ =-+*/^&!@#%(){}[]?<>,.~`'\"\\|\t\r\n;]%ln", varname, &len);
    *ptr += len;

    for (int i = 1; i <= vartable->size; i++) {
        if (listSearch((List*)(vartable->arr[vartable->size - i]), varname) == EMPTY) {
            // error
        }
    }

    return newProgNodeOpVar(varname);
}

static prog_t* getFunc (char* *ptr, Stack* vartable, List* funclist) {
    skipSpaces(ptr);
    List* var_loc = NULL;
    listCtor(var_loc, 1);
    stackPush_(vartable, var_loc);
    prog_t* ret = NULL;

    if (checklex(*)) {
        
    } else if (checklex(^)) {

    } else if (checklex(_)) {

    } else {
        ret = NULL;
    }

    void* var_loc_buf = NULL;
    stackPop_(vartable, &var_loc_buf);
    var_loc = (List*)var_loc_buf;
    // free(var_loc) & copy elems
    return ret;
}





/** TODO
 * 0. newNode....
 * 1. getEq
 * 2. precompile
 * 3. stack of lists
 * 4. funclist
 * 
 */




