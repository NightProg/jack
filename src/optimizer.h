#ifndef JACK_OPTIMIZER_H
#define JACK_OPTIMIZER_H

#include "ast.h"
#include "type.h"

typedef struct {
    StmtList *stmts;
    ScopeManager *scope_manager;
} Optimizer;

Optimizer *new_optimizer(StmtList *stmts, ScopeManager *scope_manager);
int optimize(Optimizer *optimizer);
void free_optimizer(Optimizer *optimizer);

#endif //JACK_OPTIMIZER_H
