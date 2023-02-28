#ifndef TYPE_H
#define TYPE_H  

typedef enum _boolean{
    true = 1,
    false = 0
} boolean;

/* Define the type of tree node */
typedef enum nodeType{
    TOKEN_TYPE,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_ID,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_SYMBOL,
    NOT_A_TOKEN
} NodeType;

typedef enum _basicType {
    boolType, 
    charType, 
    intType, 
    floatType, 
    voidType,
} BasicType;

typedef enum _kind{
    BASIC,
    FUNC,
    ARRAY,
    STRUCTURE,
    POINTER,
} Kind;

typedef enum _errorType{
    undef_var = 1,
    undef_func = 2,
    redef_var = 3,
    redef_func = 4,
    dismatch_assign = 5,
    rvalue_assign = 6,
    dismatch_op = 7,
    dismatch_return = 8,
    dismatch_para = 9,
    non_array = 10,
    non_func = 11,
    not_int_array_idx = 12,
    dismatch_dot = 13,
    undefined_field = 14,
    redef_struct_field = 15,
    init_struct_field = 15,
    redef_struct = 16,
    undef_struct = 17,
    only_declare_func = 18,
    dismatch_declare_func = 19, 
    nest_func_def = 20,
} ErrorType;

typedef enum _funcState{
    declared,
    defined,
} FuncState;


#endif