#ifndef SPECTYPES
#define SPECTYPES

enum CODE_TYPE {
    CODE_num  ,
    CODE_op   ,
    CODE_var  , ////
    CODE_if   ,
    CODE_while,
    CODE_for  , ////
    CODE_e    ,
    CODE_block,
    CODE_ret  ,
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

#endif