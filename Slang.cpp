#include "Slang.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

void progDump (vector<block_t> blocklist) {
    FILE* dmp = fopen("test.dot", "w"); // rewrite this sh!t
    fprintf(dmp, "digraph g {\n");
    
    for (size_t i = 0; i < blocklist.size(); i++) {
        fprintf(dmp, "\tsubgraph cluster_%ld \t {\n", i);
        fprintf(dmp, "\t\tlabel = \"block_%ld: %s \"\n\t\tcolor=blue\n",
                     i, (blocklist[i].isFunc) ? (blocklist[i].name) : ("noname"));
        fprintf(dmp, "\t\t{\n");

        fprintf(dmp, "\t\tvar_struct%ld [label=\"var: ", i);
        for (size_t j = 0; j < blocklist[i].varlist->size(); j++) {
            fprintf(dmp, "| %s ", (*blocklist[i].varlist)[j].name); // here
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
    vector<block_t> blocks;
    static stack<vector<var_t>> vartable;

    vector<var_t> varglobal;
    block_t globalblock;
    blocks.push_back(globalblock);
    vartable.push(varglobal);
    
    strCpy(blocks[0].name, "global");
    blocks[0].varlist = &vartable.top();

    blocks[0].body = NULL;
    prog_t** buf = &blocks[0].body;
    bool isWorking = true;
    while (isWorking) {
        isWorking = false;
        prog_t* newVar = getVar(ptr, &blocks[0], &(vartable.top()));

        if (newVar) {
            isWorking = true;

            *buf = newNodeEmpty(newVar, NULL);
            buf = &((*buf)->right);
        }
    }
    
    return blocks;
}

static prog_t* getVar (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    char* newVarName = startVar(ptr, block, vartable);
    if (!newVarName) return NULL;

    prog_t* eq;
    skipSpaces(ptr);
    if (checkLex("=")) {
        eq = getEq(ptr, block, vartable);
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
    int len;

    if (checkLex("$"));
    else return NULL;

    skipSpaces(ptr);
    
    sscanf(*ptr, "%[^=-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n ]%n", varname, &len); //here len = 0(?) ATTENTION
    len = strlen(varname);
    *ptr = *ptr + len;
    if (!vectorSearch(vartable, varname)) error();

    vartable->push_back(getVar_t(varname));
    return varname;
}

static prog_t* getEq (char* *ptr, block_t* block, vector<var_t>* vartable) {
    return NULL; // end
}

