#include "Slang.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static vector<block_t> Blocks;
//static vector<vector<var_t>*> Vartable;
static vector<block_t*> Blocktable;

#define strCpy(org,src) org = (char*)calloc(strlen(src) + 1, sizeof(char)); strcpy(org, src)
#define checkLex(_lex) (!strncmp(_lex, (*ptr), strlen(_lex))) && ((*ptr) += strlen(_lex))
#define error()

static var_t getVar_t (const char* name) {
    var_t ret;
    ret.name = NULL;
    strCpy(ret.name, name);
    return ret;
}

static var_t* vectorSearch (vector<var_t>* arr, char* name) {
    for (size_t i = 0; i < arr->size(); i++) {
        if (!strcmp((*arr)[i].name, name)) {
            return &((*arr)[i]);
        }
    }

    return NULL;
}

static prog_t* newNodeNum (double num) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_num;
    ret->num = num;

    return ret;
}

static prog_t* newNodeVar (char* name) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_var;
    strCpy(ret->varname, name);

    return ret;
}

static prog_t* newNodeCop (COP_TYPE type, prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_op;
    ret->op_type = type;
    ret->left  = left;
    ret->right = right;

    return ret;
}

static prog_t* newNodeCll (size_t funcind, prog_t* args) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_block;
    ret->left = args;
    ret->number = funcind;

    return ret;
}

static prog_t* newNodeEmpty (prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_e;
    ret->left  = left;
    ret->right = right;

    return ret;
}

static size_t skipSpaces (char* *ptr) {
    size_t empty = 0;
    while (strchr(" \t\r\n", **ptr)) {
        (*ptr)++;
        empty++;
    }

    return empty;
}

static void progDump_edge (prog_t* node, FILE* dmp);
static const char* dump_operation (COP_TYPE type);
static void progDump_node (prog_t* node, FILE* dmp);
vector<block_t> getProgram (char* *ptr);
static prog_t* getVar (char* *ptr, block_t* block, vector<var_t>* vartable);
static char* startVar (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getEq (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getSum (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getMul (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getPow (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getBrk (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getVarCll (char* *ptr, block_t* block, vector<var_t>* vartable);

void progDump (vector<block_t> blocklist) {
    //printf("ok\n");
    FILE* dmp = fopen("test.dot", "w"); // rewrite this sh!t
    fprintf(dmp, "digraph g {\n");
    
    for (size_t i = 0; i < blocklist.size(); i++) {
        fprintf(dmp, "\tsubgraph cluster_%ld \t {\n", i);
        fprintf(dmp, "\t\tlabel = \"block_%ld: %s \"\n\t\tcolor=blue\n",
                     i, (blocklist[i].isFunc) ? (blocklist[i].name) : ("noname"));
        fprintf(dmp, "\t\t{\n");

        fprintf(dmp, "\t\tvar_struct%ld [label=\"var: ", i);
        for (size_t j = 0; j < blocklist[i].varlist.size(); j++) {
            fprintf(dmp, "| %s ", blocklist[i].varlist[j].name); // here
        }
        fprintf(dmp, "\"]");
        
        progDump_node(blocklist[i].body, dmp);
        fprintf(dmp, "\t\t}\n");
        fprintf(dmp, "\t\tvar_struct%ld -> struct%p\n", i, blocklist[i].body);

        progDump_edge(blocklist[i].body, dmp);
        
        fprintf(dmp, "\t}\n");
    }
    fprintf(dmp, "}\n");
    fclose(dmp);
    system("dot -Tpng test.dot > pic.png");
}

static void progDump_edge (prog_t* node, FILE* dmp) {
    if (node->left)  fprintf(dmp, "\t\tstruct%p -> struct%p\n", node, node->left);
    if (node->right) fprintf(dmp, "\t\tstruct%p -> struct%p\n", node, node->right);


    if (node->left)  progDump_edge(node->left,  dmp);
    if (node->right) progDump_edge(node->right, dmp);
}

static const char* dump_operation (COP_TYPE type) {
    switch (type) {
        case COP_add:    return "+";
        case COP_assign: return "=";
        case COP_cos:    return "cos";
        case COP_cosh:   return "cosh";
        case COP_div:    return "/";
        case COP_log:    return "log";
        case COP_mul:    return "*";
        case COP_pow:    return "^";
        case COP_sin:    return "sin";
        case COP_sinh:   return "sinh";
        case COP_sub:    return "-";
        case COP_tan:    return "tan";
        case COP_tanh:   return "tanh";
    }
}  

static void progDump_node (prog_t* node, FILE* dmp) {
    switch (node->node_type) {
        case CODE_block:
            fprintf(dmp, "\t\t\tstruct%p [label=\"  \"]\n", node); // switch
            break;
        case CODE_e:
            fprintf(dmp, "\t\t\tstruct%p [label=\" E \"]\n", node);
            break;
        case CODE_num:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %lg \"]\n", node, node->num);
            break;
        case CODE_op:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %s \"]\n", node, dump_operation(node->op_type));
            break;
        case CODE_var:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %s \"]\n", node, node->varname);
            break;
    }

    if (node->left)  progDump_node(node->left,  dmp);
    if (node->right) progDump_node(node->right, dmp);
}

vector<block_t> getProgram (char* *ptr) {
    Blocks.clear();
    Blocktable.clear();

    vector<var_t> varglobal;
    block_t globalblock;
    Blocks.push_back(globalblock);

    //Vartable.push_back(&Blocks[0].varlist);
    Blocktable.push_back(&Blocks[0]);

    Blocks[0].body = NULL;
    prog_t** buf = &Blocks[0].body;
    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        prog_t* newVar = getVar(ptr, &Blocks[0], &(Blocktable.back()->varlist));
        if (newVar) {
            isWorking = true;

            *buf = newNodeEmpty(newVar, NULL);
            buf = &((*buf)->right);
        }

        prog_t* newFnc = getFnc(ptr);
    }
    
    return Blocks;
}

static prog_t* getVar (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    char* newVarName = startVar(ptr, block, vartable);
    if (!newVarName) return NULL;

    prog_t* eq = NULL;
    skipSpaces(ptr);
    if (checkLex("=")) {
        eq = getEq(ptr, block, vartable); /// PROBLEM
    } else {
        eq = newNodeNum(0);
    }
    
    skipSpaces(ptr);
    if (checkLex(";"));
    else return NULL;
    return newNodeCop(COP_assign, newNodeVar(newVarName), eq);
}

static char* startVar (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    char* varname = (char*)calloc(1000, sizeof(char));
    size_t len;

    if (checkLex("$"));
    else return NULL;

    skipSpaces(ptr);
    
    sscanf(*ptr, "%[^=-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n ]%ln", varname, &len); //here len = 0(?) ATTENTION
    len = strlen(varname);
    *ptr = *ptr + len;
    if (!vectorSearch(vartable, varname)) error();

    vartable->push_back(getVar_t(varname));
    return varname;
}

static prog_t* getEq (char* *ptr, block_t* block, vector<var_t>* vartable) {
    return getSum(ptr, block, vartable);
} 

static prog_t* getSum (char* *ptr, block_t* block, vector<var_t>* vartable) { 
    prog_t* ret = getMul(ptr, block, vartable);

    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        skipSpaces(ptr);
        if (checkLex("+")) {
            isWorking = true;

            ret = newNodeCop(COP_add, ret, getMul(ptr, block, vartable));
        }

        skipSpaces(ptr);
        if (checkLex("-")) {
            isWorking = true;

            ret = newNodeCop(COP_sub, ret, getMul(ptr, block, vartable));
        }
    }

    return ret;
}

static prog_t* getMul (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = getPow(ptr, block, vartable);

    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        skipSpaces(ptr);
        if (checkLex("*")) {
            isWorking = true;

            ret = newNodeCop(COP_mul, ret, getPow(ptr, block, vartable));
        }

        skipSpaces(ptr);
        if (checkLex("/")) {
            isWorking = true;

            ret = newNodeCop(COP_div, ret, getPow(ptr, block, vartable));
        }
    }

    return ret;
}

static prog_t* getPow (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = getBrk(ptr, block, vartable);

    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        skipSpaces(ptr);
        if (checkLex("^")) {
            isWorking = true;

            ret = newNodeCop(COP_pow, ret, getBrk(ptr, block, vartable));
        }
    }

    return ret;
}

static prog_t* getBrk (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex("(")) {
        ret = getEq(ptr, block, vartable);

        skipSpaces(ptr);
        if (checkLex(")"));
        else error();
    } else {
        ret = getVarCll(ptr, block, vartable);
    }

    return ret;
}

static prog_t* getVarCll (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    size_t len = 0;
    prog_t* ret = NULL;
    double num = 0;
    char* name = (char*)calloc(1000, sizeof(char));

    if (sscanf(*ptr, "%lg%ln", &num, &len)) {
        *ptr += len;
        return newNodeNum(num);
    }

    sscanf(*ptr, "%[^=-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n ]%ln", name, &len); // attention
    len = strlen(name);
    *ptr += len;
    skipSpaces(ptr);

    if (checkLex("(")) { // ATTENTION
        bool isDeclarated = false;
        size_t funcind = 0;

        for (size_t i = 0; i < Blocks.size(); i++) {
            if (!strcmp(Blocks[i].name, name)) {
                isDeclarated = true;
                funcind = i;
            }
        }

        if (!isDeclarated) error();

        prog_t** buf = &ret;

        for (size_t i = 0; i < Blocks[funcind].varlist.size(); i++) {
            if (i == 0) {
                skipSpaces(ptr);
                *buf = newNodeEmpty(getEq(ptr, block, vartable), NULL);
            } else {
                skipSpaces(ptr);
                if (checkLex(","));
                else error();

                skipSpaces(ptr);
                getEq(ptr, block, vartable);

                *buf = newNodeEmpty(getEq(ptr, block, vartable), NULL);
            }

            buf = &((*buf)->right);
        }   

        ret = newNodeCll(funcind, ret);

        free(name);
        if (checkLex(")")) return ret;
        else error();
    } else {
        bool isDeclarated = false;

        for (size_t i = Blocktable.size() - 1; i + 1 > 0; i--) {
            if (vectorSearch(&(Blocktable[i]->varlist), name)) {
                ret = newNodeVar(name);
                isDeclarated = true;
                ret->number  = i;
            }
        }

        free(name);
        if (!isDeclarated) error();
    }

    return ret;
}

static prog_t* getFnc (char* *ptr) {
    skipSpaces(ptr);

    prog_t* ret = NULL;
    block_t fnc = {0};
    size_t len  = 0;
    char*  name = (char*)calloc(1000, sizeof(char));

    if (checkLex("_")) {
        skipSpaces(ptr);
        strCpy(fnc.name, "_main");

        fnc.isFunc = true;
        fnc.isRet  = false;
        fnc.argnum = 0;
    } else if (checkLex("^")) {
        skipSpaces(ptr);
        sscanf(*ptr, "%[^=-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n ]%ln", name, &len); // attention
        len = strlen(name);
        *ptr += len;
        strCpy(fnc.name, name);
        free(name);
        skipSpaces(ptr);

        fnc.isFunc = true;
        fnc.isRet  = true;    
    } else if (checkLex("*")) {
        skipSpaces(ptr);
        sscanf(*ptr, "%[^=-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n ]%ln", name, &len); // attention
        len = strlen(name);
        *ptr += len;
        strCpy(fnc.name, name);
        free(name);
        skipSpaces(ptr);

        fnc.isFunc = true;
        fnc.isRet  = false;
    } else {
        free(name);
        return ret;
    }

    if (strcmp(fnc.name, "_main")) {
        // start here
        char* arg = startVar(ptr, &fnc, &fnc.varlist);

    }


    return ret;
}



