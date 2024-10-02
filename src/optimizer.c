#include "optimizer.h"

Optimizer *new_optimizer(StmtList *stmts, ScopeManager *scope_manager) {
    Optimizer *optimizer = malloc(sizeof(Optimizer));
    if (optimizer == NULL) {
        return NULL;
    }
    optimizer->stmts = stmts;
    optimizer->scope_manager = scope_manager;
    return optimizer;
}

int find_stmt_by_fn_name(StmtList *stmts, String *fn_name) {
    for (int i = 0; i < stmts->size; i++) {
        Stmt *stmt = stmts->stmts[i];
        if (stmt->type == STMT_FUNCTION) {
            if (compare_strings(stmt->function.name, fn_name)) {
                return i;
            }
        }
    }
    return -1;
}

int find_stmt_by_struct_name(StmtList *stmts, String *struct_name) {
    for (int i = 0; i < stmts->size; i++) {
        Stmt *stmt = stmts->stmts[i];
        if (stmt->type == STMT_STRUCT) {
            if (compare_strings(stmt->struc.name, struct_name)) {
                return i;
            }
        }
    }
    return -1;
}


int optimize(Optimizer *optimizer) {
    for (int i = 0; i < optimizer->scope_manager->global->scope->size; i++) {
        VarInfo *var_info = optimizer->scope_manager->global->scope->vars[i];
        if (!var_info->type.stat.num_use) {
            if (var_info->type.kind == TYPE_FUNC)  {
                remove_stmt(optimizer->stmts, find_stmt_by_fn_name(optimizer->stmts, var_info->name));
            } else if (var_info->type.kind == TYPE_STRUCT) {
                remove_stmt(optimizer->stmts, find_stmt_by_struct_name(optimizer->stmts, var_info->name));
            }
        }
    }
    return 0;
}

void free_optimizer(Optimizer *optimizer) {
    free_stmt_list(optimizer->stmts);
    free(optimizer);
}
