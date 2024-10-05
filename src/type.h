#ifndef JACK_TYPE_H
#define JACK_TYPE_H

#include "ast.h"
#include "error.h"
#include "gc.h"


typedef struct TypeChecker TypeChecker;

// type checking

typedef int (ExprRuleFunc)(TypeChecker *tc, Expr *expr);

typedef struct {
    ExprType type;
    ExprRuleFunc *rule;
} ExprRule;

ExprRule *new_expr_rule(ExprType type, ExprRuleFunc *rule);

typedef struct {
    ExprRule **rules;
    size_t size;
    size_t capacity;
} ExprRules;

ExprRules *new_expr_rules();
int append_expr_rule(ExprRules *rules, ExprRule *rule);
void free_expr_rules(ExprRules *rules);

typedef int (StmtRuleFunc)(TypeChecker *tc, Stmt *stmt);

typedef struct {
    StmtType type;
    StmtRuleFunc *rule;
} StmtRule;

StmtRule *new_stmt_rule(StmtType type, StmtRuleFunc *rule);

typedef struct {
    StmtRule **rules;
    size_t size;
    size_t capacity;
} StmtRules;

StmtRules *new_stmt_rules();
int append_stmt_rule(StmtRules *rules, StmtRule *rule);
void free_stmt_rules(StmtRules *rules);

typedef struct {
    String *name;
    Type type;
} VarInfo;

typedef struct {
    VarInfo **vars;
    size_t size;
    size_t capacity;
} VarInfoList;

VarInfoList *new_var_info_list();
int append_var_info(VarInfoList *list, VarInfo *var_info);
void free_var_info_list(VarInfoList *list);

typedef struct {
    VarInfoList *scope;
    struct Scope *parent;
} Scope;


Scope *new_scope(Scope *parent);
void free_scope(Scope *scope);

VarInfo *new_var_info(String *name, Type type);
int add_var_info(Scope *scope, VarInfo *var_info);
VarInfo *get_var_info(Scope *scope, String *name);

typedef struct {
    Scope *global;
    Scope *current;
} ScopeManager;


ScopeManager *new_scope_manager();
int enter_scope(ScopeManager *manager);
int exit_scope(ScopeManager *manager);
void free_scope_manager(ScopeManager *manager);

int add_var(ScopeManager *manager, String *name, Type type);
Type* get_var(ScopeManager *manager, String *name);

struct TypeChecker {
    ScopeManager *scope_manager;
    StmtRules *stmt_rules;
    ExprRules *expr_rules;
    String *source;
    String *current_function;
    Type should_return;
    String *current_struct;
    Symbols *symbols;
    StringList *module_paths;
    StmtList *stmts;
    Expr *expr_get_parent;
    Type *parent_type;
    ExtensionList *extensions;
};

TypeChecker *new_type_checker(Symbols *symbols, StmtList *stmts, const char *source);

int check_expr(TypeChecker *tc, Expr *expr);
int check_stmt(TypeChecker *tc, Stmt *stmt);
int check_tc(TypeChecker *tc);
int check_same_type(Type *type1, Type *type2);




#endif //JACK_TYPE_H
