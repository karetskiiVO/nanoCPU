#include "Slang.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static vector<block_t> Blocks; // vector of blocks
//static vector<vector<var_t>*> Vartable;
static vector<size_t> Blocktable; // stack of now blocks

#define debugprint printf("%s\t%s\n", __PRETTY_FUNCTION__, *ptr)
#define strCpy(org,src) org = (char*)calloc(strlen(src) + 1, sizeof(char)); strcpy(org, src)
#define checkLex(_lex) (!strncmp(_lex, (*ptr), strlen(_lex))) && ((*ptr) += strlen(_lex))
#define error()

static int scanString (char* *origin, char* str, const char* ignore) {
    size_t len = strpbrk(*origin, ignore) - *origin;
    strncpy(str, *origin, len);
    str[len] = '\0';

    (*origin) += len;

    if (len) return 1;
    return 0;
}

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

static prog_t* newNodeCode (CODE_TYPE type, prog_t* left, prog_t* right) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = type;
    ret->left  = left;
    ret->right = right;

    return ret;
}

static prog_t* newNodeCll (size_t funcind, prog_t* args) {
    prog_t* ret = (prog_t*)calloc(1, sizeof(prog_t));
    ret->node_type = CODE_block;
    ret->left = args;
    ret->ind = funcind;

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
static vector<block_t> getProgramm (char* *ptr);
static prog_t* getVar (char* *ptr, block_t* block, vector<var_t>* vartable);
static char* startVar (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getEq (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getAsgn (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getSum (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getMul (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getPow (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getBrk (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getVarCll (char* *ptr, block_t* block, vector<var_t>* vartable);
static bool getFnc (char* *ptr);
static prog_t* getLine (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getEqL (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getBlock (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getIf (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getWhile (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getRet (char* *ptr, block_t* block, vector<var_t>* vartable);
static prog_t* getInOut (char* *ptr, block_t* block, vector<var_t>* vartable);

void progDump (vector<block_t> blocklist) {
    FILE* dmp = fopen("test.dot", "w"); // rewrite this sh!t
    fprintf(dmp, "digraph g {\n\tnode [shape=record]\n");
    
    for (size_t i = 0; i < blocklist.size(); i++) {
        fprintf(dmp, "\tsubgraph cluster_%ld \t {\n", i);
        fprintf(dmp, "\t\tlabel = \"block_%ld: %s \"\n\t\tcolor=blue\n",
                     i, (blocklist[i].isFunc) ? (blocklist[i].name) : ("noname"));
        fprintf(dmp, "\t\t{\n");

        fprintf(dmp, "\t\tvar_struct%ld [label=\"var: ", i);
        for (size_t j = 0; j < blocklist[i].varlist.size(); j++) {
            fprintf(dmp, "| %s ", blocklist[i].varlist[j].name); // here
        }
        fprintf(dmp, "\"]\n");
        
        progDump_node(blocklist[i].body, dmp);
        fprintf(dmp, "\t\t}\n");
        if (blocklist[i].body) fprintf(dmp, "\t\tvar_struct%ld -> struct%p\n", i, blocklist[i].body);
        progDump_edge(blocklist[i].body, dmp);
        
        fprintf(dmp, "\t}\n");
    }
    fprintf(dmp, "}\n");
    fclose(dmp);
    system("dot -Tpng test.dot > pic.png");
}

static void progDump_edge (prog_t* node, FILE* dmp) {
    if (!node) return;

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

    return NULL;
}  

static void progDump_node (prog_t* node, FILE* dmp) {
    if (!node) return;

    switch (node->node_type) {
        case CODE_block:
            fprintf(dmp, "\t\t\tstruct%p [label=\" block_%ld \"", node, node->ind); // switch
            break;
        case CODE_e:
            fprintf(dmp, "\t\t\tstruct%p [label=\" E \"", node);
            break;
        case CODE_num:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %lg \"", node, node->num);
            break;
        case CODE_op:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %s \"", node, dump_operation(node->op_type));
            break;
        case CODE_var:
            fprintf(dmp, "\t\t\tstruct%p [label=\" %s \"", node, node->varname);
            break;
        case CODE_if:
            fprintf(dmp, "\t\t\tstruct%p [label=\" if \"", node);
            break;
        case CODE_while:
            fprintf(dmp, "\t\t\tstruct%p [label=\" while \"", node);
            break;
        case CODE_ret:
            fprintf(dmp, "\t\t\tstruct%p [label=\" return \"", node);
            break;
        case CODE_in:
            fprintf(dmp, "\t\t\tstruct%p [label=\" in \"", node);
            break;
        case CODE_out:
            fprintf(dmp, "\t\t\tstruct%p [label=\" out \"", node);
            break;
    }
    fprintf(dmp, " color=\"olivedrab1\"]\n");

    if (node->left)  progDump_node(node->left,  dmp);
    if (node->right) progDump_node(node->right, dmp);
}

static void seekFnc (char* *ptr) {
    char* name = (char*)calloc(1000, sizeof(char));
    while (**ptr != '\0') {
        block_t fnc = {0};

        if (checkLex("_")) {
            skipSpaces(ptr);
            strcpy(name, "_main");

            fnc.isFunc = true;
            fnc.isRet  = false;
            fnc.argnum = 0;
        } else if (checkLex("^")) {
            skipSpaces(ptr);
            scanString(ptr, name, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n");
            skipSpaces(ptr);

            fnc.isFunc = true;
            fnc.isRet  = true;    
        } else if (checkLex("*")) {
            skipSpaces(ptr);
            scanString(ptr, name, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n");
            skipSpaces(ptr);

            fnc.isFunc = true;
            fnc.isRet  = false;
        } else {
            (*ptr)++;
            continue;
        }

        bool isDeclarated = false;
        strCpy(fnc.name, name);
        for (size_t i = 0; i < Blocks.size(); i++) {
            if (Blocks[i].isFunc && !strcmp(Blocks[i].name, fnc.name)) {
                isDeclarated = true;
            }
        }
        if (isDeclarated) error();
        

        if (checkLex("(")) ;
        else {
            error();
        }

        bool isWorking = true;
        while (isWorking) {
            skipSpaces(ptr);
            if (checkLex(")")) {
                isWorking = false;
                break;
            } else if (checkLex(",")) {
                skipSpaces(ptr);
                fnc.argnum++;
                *ptr = strpbrk(*ptr ,",)");
            } else if (checkLex("$")) {
                fnc.argnum++;
                skipSpaces(ptr);
                *ptr = strpbrk(*ptr ," =-+*/$;&!@#%(){}[]?<>,.~`'\"\t\r\n");
            } else {
                error();
            }
        }
        
        
        skipSpaces(ptr);
        if (checkLex("{")) {
            int num = 1;
            while (num) {
                if (**ptr == '\0'){
                    error();
                    break;
                }

                if (checkLex("{")) {
                    num++;
                } else if (checkLex("}")) {
                    num--;
                } else {
                    (*ptr)++;
                }     
            }
        } else {
            error();
        }
        Blocks.push_back(fnc);
    }
}

vector<block_t> Compile (char* code) {
    vector<var_t> varglobal;
    block_t globalblock;
    globalblock.isFunc = false;
    globalblock.isRet  = false;
    globalblock.argnum = 0;
    strCpy(globalblock.name, "global");
    Blocks.push_back(globalblock);

    char* buf = code;
    seekFnc(&code);
    return getProgramm(&buf);
}

static vector<block_t> getProgramm (char* *ptr) {
    //Vartable.push_back(&Blocks[0].varlist);
    Blocktable.push_back(0);

    Blocks[0].body = NULL;
    prog_t** buf = &Blocks[0].body;
    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        prog_t* newVar = getVar(ptr, &Blocks[0], &(Blocks[Blocktable.back()].varlist));
        if (newVar) {
            isWorking = true;

            *buf = newNodeEmpty(newVar, NULL);
            buf = &((*buf)->right);
        }

        if (getFnc(ptr)) {
            isWorking = true;
        }
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

    if (checkLex("$"));
    else return NULL;

    skipSpaces(ptr);
    
    scanString(ptr, varname, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n");
    if (!vectorSearch(&(Blocks[Blocktable.back()].varlist), varname)) error();

    Blocks[Blocktable.back()].varlist.push_back(getVar_t(varname));
    //vartable->push_back(getVar_t(varname));
    return varname;
}

static prog_t* getEq (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = getAsgn(ptr, block, vartable);
    return ret;
} 

static prog_t* getAsgn (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = getSum(ptr, block, vartable);

    bool isWorking = true;
    while (isWorking) {
        isWorking = false;

        skipSpaces(ptr);
        if (checkLex("=")) {
            isWorking = true;

            ret = newNodeCop(COP_assign, ret, getSum(ptr, block, vartable));
        }
    }

    return ret;
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
        else {
            error();
        }
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

    if (!scanString(ptr, name, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n")) return NULL;
    skipSpaces(ptr);

    if (checkLex("(")) { // ATTENTION
        bool isDeclarated = false;
        size_t funcind = 0;

        for (size_t i = 0; i < Blocks.size(); i++) {
            if (Blocks[i].isFunc && !strcmp(Blocks[i].name, name)) {
                isDeclarated = true;
                funcind = i;
            }
        }

        if (!isDeclarated) error();

        prog_t** buf = &ret;

        for (size_t i = 0; i < Blocks[funcind].argnum; i++) {
            if (i == 0) {
                skipSpaces(ptr);
                *buf = newNodeEmpty(getEq(ptr, block, vartable), NULL);
            } else {
                skipSpaces(ptr);
                if (checkLex(","));
                else {
                    error();
                }

                skipSpaces(ptr);

                *buf = newNodeEmpty(getEq(ptr, block, vartable), NULL);
            }

            buf = &((*buf)->right);
        }   

        ret = newNodeCll(funcind, ret);

        free(name);
        if (checkLex(")")) return ret;
        else {
            error();
        }
    } else { // PROBLEM is HERE
        bool isDeclarated = false;

        /*printf("%s\n", name);
        for (size_t i = 0; i < Blocktable.size(); i++) {
            printf("%ld\t", Blocktable[i]->varlist.size());
        }
        printf("\n");*/

        for (size_t i = 0; i < Blocktable.size(); i++) {
            if (vectorSearch(&(Blocks[Blocktable[Blocktable.size() - i - 1]].varlist), name)) {
                ret = newNodeVar(name);
                isDeclarated = true;
                ret->ind  = i;
            }
        }
        
        free(name);
        if (!isDeclarated) error();
    }

    return ret;
}

static bool getFnc (char* *ptr) {
    skipSpaces(ptr);
    block_t fnc = {0};
    char*  name = (char*)calloc(1000, sizeof(char));

    if (checkLex("_")) {
        skipSpaces(ptr);
        strcpy(name, "_main");

        fnc.isFunc = true;
        fnc.isRet  = false;
        fnc.argnum = 0;
    } else if (checkLex("^")) {
        skipSpaces(ptr);
        scanString(ptr, name, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n");
        skipSpaces(ptr);
        
        fnc.isFunc = true;
        fnc.isRet  = true;    
    } else if (checkLex("*")) {
        skipSpaces(ptr);
        scanString(ptr, name, "^ ,.=-+*/$;&!@#%(){}[]?<>~`'\"\t\r\n");
        skipSpaces(ptr);

        fnc.isFunc = true;
        fnc.isRet  = false;
    } else {
        free(name);
        return false;
    }

    strCpy(fnc.name, name);

    size_t funcind = 0;
    for (size_t i = 0; i < Blocks.size(); i++) {
        if (Blocks[i].isFunc && !strcmp(Blocks[i].name, fnc.name)) {
            funcind = i;
        }
    }
    
    if (!funcind) error();

    skipSpaces(ptr);
    if (checkLex("(")) ;
    else {
        error();
    }

    for (size_t i = 0; i < Blocks[funcind].argnum; i++) {
        if (i != 0) {
            skipSpaces(ptr);
            if (checkLex(",")) ;
            else {
            error();
        }
        }

        if (startVar(ptr, &Blocks[funcind], &Blocks[funcind].varlist));
        else {
            error();
        }
        
        skipSpaces(ptr);
    }

    skipSpaces(ptr);
    if (checkLex(")")) ;
    else {
        error();
    }

    Blocktable.push_back(funcind);

    skipSpaces(ptr);
    if (checkLex("{")) ;
    else {
        error();
    }

    bool isWorking = true;
    prog_t** buf = &Blocks[funcind].body;

    while (isWorking) {
        isWorking = false;

        *buf = getLine(ptr, &Blocks[funcind], &Blocks[funcind].varlist);
        if (*buf) {
            isWorking = true;
        } else {
            break;
        }

        *buf = newNodeEmpty(*buf, NULL);
        buf = &((*buf)->right);
    }

    skipSpaces(ptr);
    if (checkLex("}")) ;
    else {
        error();
    }

    Blocktable.pop_back();

    return true;
}

static prog_t* getLine (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = NULL;

    ret = getVar(ptr, block, vartable);
    if (ret) return ret;

    ret = getEqL(ptr, block, vartable);
    if (ret) return ret;

    ret = getBlock(ptr, block, vartable);
    if (ret) return ret;

    ret = getIf(ptr, block, vartable);
    if (ret) return ret;

    ret = getWhile(ptr, block, vartable);
    if (ret) return ret;

    ret = getRet(ptr, block, vartable);
    if (ret) return ret;

    ret = getInOut(ptr, block, vartable);
    if (ret) return ret;

    return NULL;
}

static prog_t* getEqL (char* *ptr, block_t* block, vector<var_t>* vartable) {
    prog_t* ret = getEq(ptr, block, vartable);

    skipSpaces(ptr);
    if (checkLex(";")) ;
    else {
        error();
    }

    return ret;
}

static prog_t* getBlock (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex("{")) {
        block_t newBlock = {0};
        newBlock.isFunc = false;
        newBlock.isRet  = false;
        newBlock.name   = NULL;
        newBlock.varlist.begin();

        Blocks.push_back(newBlock);
        size_t blockind = Blocks.size() - 1;
        Blocktable.push_back(blockind);

        bool isWorking = true;
        prog_t** buf = &Blocks[blockind].body;

        while (isWorking) {
            isWorking = false;
            prog_t* line = getLine(ptr, &Blocks[blockind], &Blocks[blockind].varlist);
            if (line) {
                isWorking = true;
            } else {
                break;
            }
            *buf = newNodeEmpty(line, NULL);
            buf = &((*buf)->right);
        }

        if (checkLex("}")) ;
        else {
            error();
        }

        Blocktable.pop_back();
        ret = newNodeCll(blockind, 0);
    }
    
    return ret;
}

static prog_t* getIf (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex("?")) {
        skipSpaces(ptr);
        if (checkLex("(")) ;
        else {
            error();
        }

        ret = newNodeCode(CODE_if, getEq(ptr, block, vartable), NULL);

        skipSpaces(ptr);
        if (checkLex(")")) ;
        else {
            error();
        }

        prog_t* buf = getLine(ptr, block, vartable);
        ret->right = newNodeEmpty(buf, NULL);
        
        skipSpaces(ptr);
        if (checkLex(":")) {
            ret->right->right = getLine(ptr, block, vartable);
        }
    }

    return ret;
}

static prog_t* getWhile (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex("~")) {
        skipSpaces(ptr);
        if (checkLex("(")) ;
        else {
            error();
        }

        ret = newNodeCode(CODE_while, getEq(ptr, block, vartable), NULL);

        skipSpaces(ptr);
        if (checkLex(")")) ;
        else {
            error();
        }

        ret->right = getLine(ptr, block, vartable);
    }

    return ret;
}

static prog_t* getRet (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex("^")) {
        ret = newNodeCode(CODE_ret, NULL, NULL);

        if (Blocks[Blocktable[1]].isRet) {
            ret->left = getEq(ptr, block, vartable);
        }
        
        skipSpaces(ptr);
        if (checkLex(";")) ;
        else {
            error();
        }
    }

    return ret;
}

static prog_t* getInOut (char* *ptr, block_t* block, vector<var_t>* vartable) {
    skipSpaces(ptr);
    prog_t* ret = NULL;

    if (checkLex(">>")) {
        ret = newNodeCode(CODE_in, getEq(ptr, block, vartable), NULL);

        skipSpaces(ptr);
        if (checkLex(";")) ;
        else {
            error();
        }

        return ret;
    }

    if (checkLex("<<")) {
        ret = newNodeCode(CODE_out, getEq(ptr, block, vartable), NULL);

        skipSpaces(ptr);
        if (checkLex(";")) ;
        else {
            error();
        }

        return ret;
    }

    return ret;
}
