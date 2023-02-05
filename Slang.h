#ifndef sLANG
#define sLANG

#include <stdbool.h>
#include <vector>

using std::vector;

enum CODE_TYPE {
    CODE_num  ,
    CODE_op   ,
    CODE_var  , ////
    CODE_if   ,
    CODE_while,
    CODE_e    ,
    CODE_block,
    CODE_ret  ,
    CODE_in   ,
    CODE_out  ,
};

enum COP_TYPE {
    COP_sin ,
    COP_cos ,
    COP_log ,
    COP_tan ,
    COP_sinh,
    COP_cosh,
    COP_tanh,
    COP_add ,
    COP_div ,
    COP_mul ,
    COP_sub ,
    COP_pow ,
    COP_assign,
};

struct prog_t;
struct var_t;

typedef struct block_t {
    prog_t* body;
    vector<var_t> varlist;

    char* name;
    bool isFunc;
    bool isRet;
    size_t argnum;
} block_t;

typedef struct prog_t {
    double num;
    char* varname;
    size_t ind;
    CODE_TYPE node_type;
    COP_TYPE op_type;

    prog_t* left;
    prog_t* right;
} prog_t;

typedef struct var_t {
    char* name;
} var_t;

void progDump (vector<block_t> blocklist);

vector<block_t> Compile (char* code);

#endif