#ifndef JACK_AST_H
#define JACK_AST_H

#define BINOP_ADD 1
#define BINOP_SUB 2
#define BINOP_MUL 3
#define BINOP_DIV 4
#define BINOP_EQ 5
#define BINOP_NEQ 6
#define BINOP_LT 7
#define BINOP_GT 8
#define BINOP_LTE 9
#define BINOP_GTE 10

#define SYMBOLS_MAGIC 0x5f4c4f42
#define SYMBOLS_VERSION 1


#define UNOP_NEG 1
#define UNOP_NOT 2

#include <stddef.h>
#include "string.h"
#include "span.h"

typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct Stmt Stmt;
typedef struct StmtList StmtList;
typedef struct Type Type;
typedef struct Param Param;
typedef struct Method Method;
typedef struct MethodParam MethodParam;
typedef struct MethodList MethodList;

typedef struct Symbol Symbol;

struct Symbol {
    String *name;
    Type *type;
    int is_extern;
    Stmt *stmt;
};

Symbol *new_symbol(String *name, Type *type, int is_extern, Stmt *stmt);
void free_symbol(Symbol *symbol);

typedef struct {
    Symbol **symbols;
    size_t size;
    size_t capacity;
} SymbolList;

SymbolList *new_symbol_list();
int append_symbol(SymbolList *list, Symbol *symbol);
Symbol *find_symbol(SymbolList *list, String *name);
void free_symbol_list(SymbolList *list);

typedef struct {
    size_t magic;
    size_t version;
    SymbolList *symbols;
} Symbols;

Symbols *new_symbols();
int append_symbols(Symbols *symbols, Symbol *symbol);
Symbol *find_symbols(Symbols *symbols, String *name);
void free_symbols(Symbols *symbols);
Symbols *read_symbols(const char *filename);
Symbols *get_symbols_from_stmts(StmtList *stmts);
Symbols *merge_symbols(Symbols *symbols1, Symbols *symbols2);
int write_symbols(Symbols *symbols, const char *filename);

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_FUNC,
    TYPE_STRUCT,
    TYPE_ARRAY,
    TYPE_PTR,
    TYPE_NULL,
    TYPE_VOID,
    TYPE_MODULE,
    TYPE_INVALID,
} TypeKind;

struct Type {
    TypeKind kind;
    Type* parent;
    union {
        struct {
            Type *params;
            int num_params;
            Type* ret;
            int is_vararg;
            String *name;
            int is_extern;
        } func;
        struct {
            String *name;
            Param *fields;
            int num_fields;
            MethodList *methods;
        } struc;
        struct {
            Type *inner_type;
            int length;
        } array;
        struct {
            String *name;
            SymbolList *symbols;
        } module;
        Type *inner_type;

    };
};

Type *new_type(TypeKind kind);
Type *new_func_type(Type *params, int num_params, Type *ret, int is_var_arg, String *name, int is_extern);
Type *new_struct_type(String *name, Param *fields, int num_fields, MethodList *methods);
Type *new_ptr_type(Type *inner_type);
Type *new_array_type(Type *inner_type, int length);
Type *new_module_type(SymbolList *symbols, String *name);
void free_type(Type *type);

typedef struct {
    Type **types;
    size_t size;
    size_t capacity;
} GenericList;

GenericList *new_generic_list();
void append_generic(GenericList *list, Type *type);
GenericList *generic_list_from_array(int length, Type **array);
void free_generic_list(GenericList *list);


struct ExprList {
    Expr **exprs;
    size_t size;
    size_t capacity;
};

ExprList *new_expr_list();
void append_expr(ExprList *list, Expr *expr);
void append_expr_at_first(ExprList *list, Expr *expr);
ExprList *expr_list_from_array(int length, Expr **array);
int remove_expr(ExprList *list, int index);
void free_expr_list(ExprList *list);

typedef enum {
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STRING,
    EXPR_CHAR,
    EXPR_IDENT,
    EXPR_BINOP,
    EXPR_UNOP,
    EXPR_CALL,
    EXPR_INIT,
    EXPR_GET,
    EXPR_ASSIGN,
    EXPR_REF,
    EXPR_DEREF,
    EXPR_AS,
    EXPR_ARRAY,
    EXPR_INDEX,
    EXPR_NULL,
    EXPR_NS,
} ExprType;

struct Expr {
    ExprType type;
    Span span;
    Type *ty;
    union {
        int int_val;
        float float_val;
        char char_val;
        String *string_val;
        String *ident_val;
        struct {
            int op;
            Expr *lhs;
            Expr *rhs;
        } binop;
        struct {
            int op;
            Expr *expr;
        } unop;
        struct {
            Expr *name;
            ExprList *args;
        } call;
        struct {
            Expr *name;
            ExprList *fields;
        } init;
        struct {
            Expr *expr;
            String *field;
        } get;
        struct {
            Expr *var;
            Expr *val;
        } assign;
        struct {
            Expr *expr;
            Type ty;
        } as;
        struct {
            ExprList *exprs;
        } array;
        struct {
            Expr *list;
            Expr *index;
        } index;
        struct {
            StringList *path;
        } ns;

        Expr *ref;
        Expr *deref;
    };
};

Expr *new_int_expr(int val, Span span);
Expr *new_float_expr(float val, Span span);
Expr *new_string_expr(String *val, Span span);
Expr *new_char_expr(char val, Span span);
Expr *new_ident_expr(String *val, Span span);
Expr *new_binop_expr(int op, Expr *lhs, Expr *rhs, Span span);
Expr *new_unop_expr(int op, Expr *expr, Span span);
Expr *new_call_expr(Expr *name, ExprList *args, Span span);
Expr *new_init_expr(Expr *name, ExprList *fields, Span span);
Expr *new_get_expr(Expr *expr, String *field, Span span);
Expr *new_assign_expr(Expr *var, Expr *val, Span span);
Expr *new_ref_expr(Expr *expr, Span span);
Expr *new_deref_expr(Expr *expr, Span span);
Expr *new_null_expr(Span span);
Expr *new_as_expr(Expr *expr, Type ty, Span span);
Expr *new_array_expr(ExprList *exprs, Span span);
Expr *new_index_expr(Expr *list, Expr *index, Span span);
Expr *new_ns_expr(StringList *path, Span span);
void free_expr(Expr *expr);


struct StmtList {
    struct Stmt **stmts;
    size_t size;
    size_t capacity;
};

StmtList *new_stmt_list();
void append_stmt(StmtList *list, struct Stmt *stmt);
StmtList *stmt_list_from_array(int length, struct Stmt **array);
int remove_stmt(StmtList *list, int index);
void free_stmt_list(StmtList *list);

struct Param {
    String* name;
    Type ty;
};

typedef enum {
    STMT_EXPR,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_BLOCK,
    STMT_RETURN,
    STMT_LET,
    STMT_FUNCTION,
    STMT_STRUCT,
    STMT_EXTERN,
    STMT_MODULE,
    STMT_IMPORT,
} StmtType;

struct Stmt {
    StmtType type;
    Span span;
    union {
        Expr *expr;
        struct {
            Expr *cond;
            Stmt *body;
            Stmt *else_body;
        } if_stmt;
        struct {
            Expr *cond;
            struct Stmt *body;
        } while_stmt;
        struct {
            Expr *init;
            Expr *cond;
            Expr *update;
            struct Stmt *body;
        } for_stmt;
        struct {
            StmtList *stmts;
        } block;
        struct {
            String *name;
            Type *type;
            Expr *expr;
        } let;
        struct {
            String *name;
            Param *args;
            Type ret;
            int num_params;
            Stmt *body;
            GenericList *generics;
        } function;
        Expr *ret_expr;
        struct {
            String *name;
            Param *args;
            int num_params;
            MethodList *methods;
        } struc;
        struct {
            String *name;
            Param *args;
            Type ret;
            int num_params;
            int is_vararg;
        } extern_;
        struct {
            String *name;
            StmtList *stmts;
        } module;
        struct {
            String *name;
        } import;
    };
};

struct Method {
    String *name;
    MethodParam *args;
    int num_params;
    Type ret;
    Stmt *body;
};

Method *new_method(String *name, MethodParam *args, int num_params, Type ret, Stmt *body);

struct MethodList {
    Method **methods;
    size_t size;
    size_t capacity;
};

MethodList *new_method_list();
void append_method(MethodList *list, Method *method);
MethodList *method_list_from_array(int length, Method **array);
void free_method_list(MethodList *list);


struct MethodParam {
    int is_self;
    String *name;
    Type ty;
};



Stmt *new_expr_stmt(Expr *expr, Span span);
Stmt *new_if_stmt(Expr *cond, Stmt *body, Span span, Stmt *else_body);
Stmt *new_while_stmt(Expr *cond, Stmt *body, Span span);
Stmt *new_for_stmt(Expr *init, Expr *cond, Expr *update, Stmt *body, Span span);
Stmt *new_block_stmt(StmtList *stmts, Span span);
Stmt *new_return_stmt(Expr *expr, Span span);
Stmt *new_function_stmt(String *name, GenericList *list, Param *param, int num_params, Stmt *body, Span span, Type ret);
Stmt *new_let_stmt(String *name, Type *type, Span span, Expr *expr);
Stmt *new_struct_stmt(String *name, Param *args, int num_params, MethodList *methods, Span span);
Stmt *new_extern_stmt(String *name, Param *args, int num_params, Type ret, int is_vararg, Span span);
Stmt *new_module_stmt(String *name, StmtList *stmts, Span span);
Stmt *new_import_stmt(String *name, Span span);
void free_stmt(Stmt *stmt);



#endif //JACK_AST_H
