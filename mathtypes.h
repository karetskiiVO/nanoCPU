#ifndef MATHTYPES
#define MATHTYPES

typedef int var_ind;
typedef struct var_t {
    char* name;
    double val;
};

enum NODE_TYPE {
    NODE_OPR,
    NODE_CST,
    NODE_VAR,
    NODE_EMPTY
};

enum OP_TYPE {
    OP_sin,
    OP_cos,
    OP_log,
    OP_tan,
    OP_sinh,
    OP_cosh,
    OP_tanh,
    OP_add = 10,
    OP_div = 20,
    OP_mul = 21,
    OP_sub = 11,
    OP_pow = 30,
    OP_EMPTY
};

#endif