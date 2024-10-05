#include "type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "mangler.h"


Type* visit_new_type(TypeChecker *tc, Type *type) {
    if (tc->parent_type) {
        *type->parent = *tc->parent_type;
    }
    return type;
}

int analyze_type(TypeChecker *tc, Type *type) {
    if (type->kind == TYPE_STRUCT) {
        Type* struct_type = get_var(tc->scope_manager, type->struc.name);
        if (struct_type == NULL) {
            add_error(errorList, "Struct not found", span(0,0,0, new_string(""), new_string("")), tc->source);
            return 0;
        }
        *type = *struct_type;
    } else if (type->kind == TYPE_FUNC) {
        for (int i = 0; i < type->func.num_params; i++) {
            if (!analyze_type(tc, &type->func.params[i])) {
                return 0;
            }
        }
        if (!analyze_type(tc, type->func.ret)) {
            return 0;
        }
    } else if (type->kind == TYPE_PTR) {
        if (!analyze_type(tc, type->inner_type)) {
            return 0;
        }
    }
    return 0;
}


int can_be_cmp(Type *lhs, Type *rhs) {
    if (lhs->kind == TYPE_PTR && rhs->kind == TYPE_NULL) {
        return 1;
    }
    if (lhs->kind == TYPE_NULL && rhs->kind == TYPE_PTR) {
        return 1;
    }
    if (lhs->kind != rhs->kind) {
        return 0;
    }
    if (lhs->kind == TYPE_INT || lhs->kind == TYPE_FLOAT || lhs->kind == TYPE_CHAR || lhs->kind == TYPE_STRING) {
        return 1;
    }
    return 0;
}

int check_same_type(Type *a, Type *b) {
    if (a->kind == TYPE_VOID) {
        return 1;
    }
    if (b->kind == TYPE_VOID) {
        return 1;
    }
    if (a->kind == TYPE_PTR && b->kind == TYPE_NULL) {
        return 1;
    }
    if (a->kind == TYPE_NULL && b->kind == TYPE_PTR) {
        return 1;
    }
    if (a->kind != b->kind) {
        return 0;
    }
    if (a->kind == TYPE_PTR) {
        return check_same_type(a->inner_type, b->inner_type);
    }
    if (a->kind == TYPE_FUNC) {
        if (a->func.num_params != b->func.num_params) {
            return 0;
        }
        for (int i = 0; i < a->func.num_params; i++) {
            if (!check_same_type(&a->func.params[i], &b->func.params[i])) {
                return 0;
            }
        }
        return a->func.ret == b->func.ret;
    }
    if (a->kind == TYPE_STRUCT) {
        if (a->struc.num_fields != b->struc.num_fields) {
            return 0;
        }
        for (int i = 0; i < a->struc.num_fields; i++) {
            if (!check_same_type(&a->struc.fields[i].ty, &b->struc.fields[i].ty)) {
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

int int_rule(TypeChecker *tc, Expr *expr) {
    *expr->ty = *new_type(TYPE_INT);
    return 1;
}

int float_rule(TypeChecker *tc, Expr *expr) {
    *expr->ty = *new_type(TYPE_FLOAT);
    return 1;
}

int string_rule(TypeChecker *tc, Expr *expr) {
    *expr->ty = *new_ptr_type(new_type(TYPE_CHAR));
    return 1;
}

int char_rule(TypeChecker *tc, Expr *expr) {
    *expr->ty = *new_type(TYPE_CHAR);
    return 1;
}

int ident_rule(TypeChecker *tc, Expr *expr) {
    Type *var_info = get_var(tc->scope_manager, expr->ident_val);
    if (var_info == NULL) {
        printf("Variable not found: %s\n", expr->ident_val->data);
        add_error(errorList, "Variable not found", expr->span, tc->source);
        return 0;
    }
    *expr->ty = *var_info;
    return 1;
}

int binop_rule(TypeChecker *tc, Expr *expr) {

    if (!check_expr(tc, expr->binop.lhs) || !check_expr(tc, expr->binop.rhs)) {
        return 0;
    }
    if (expr->binop.lhs->ty->kind == TYPE_STRUCT) {
        OpOverload *op = find_op_overload(expr->binop.lhs->ty->struc.op_overloads, OP_OVERLOAD_BINOP, expr->binop.op);
        if (op == NULL) {
            add_error(errorList, "Operator not found", expr->span, tc->source);
            return 0;
        }

        if (!check_same_type(expr->binop.rhs->ty, &op->method->args[1].ty)) {
            add_error(errorList, "Types do not match", expr->span, tc->source);
            return 0;
        }
        *expr->ty = op->method->ret;
        return 1;
    }
    if (expr->binop.op == BINOP_EQ || expr->binop.op == BINOP_NEQ || expr->binop.op == BINOP_LT || expr->binop.op == BINOP_LTE || expr->binop.op == BINOP_GT || expr->binop.op == BINOP_GTE) {
        if (!can_be_cmp(expr->binop.lhs->ty, expr->binop.rhs->ty)) {
            add_error(errorList, "Invalid type: Expected int, float, char or string", expr->span, tc->source);
            return 0;
        }
        *expr->ty = *new_type(TYPE_BOOL);
        return 1;
    }
    if (expr->binop.lhs->ty->kind == TYPE_INT && expr->binop.rhs->ty->kind == TYPE_INT) {
        *expr->ty = *new_type(TYPE_INT);
    } else if (expr->binop.lhs->ty->kind == TYPE_FLOAT && expr->binop.rhs->ty->kind == TYPE_FLOAT) {
        *expr->ty = *new_type(TYPE_FLOAT);
    } else if (expr->binop.lhs->ty->kind == TYPE_PTR && expr->binop.rhs->ty->kind == TYPE_INT) {
        *expr->ty = *expr->binop.lhs->ty;
    } else {
        add_error(errorList, "Invalid type: Expected int or float", expr->span, tc->source);
        return 0;
    }

    return 1;
}

int call_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->call.name)) {
        return 0;
    }
    if (expr->call.name->ty->kind != TYPE_FUNC) {
        add_error(errorList, "Expected function", expr->call.name->span, tc->source);
        return 0;
    }

    if (expr->call.name->type == EXPR_GET) {
        append_expr_at_first(expr->call.args, tc->expr_get_parent);
    } else if (expr->call.name->type == EXPR_IDENT) {
        Type *var_info = get_var(tc->scope_manager, expr->call.name->ident_val);
        var_info->stat.num_use++;
    }

//    if (var_info->type.func.is_vararg) {
//        if (var_info->type.func.num_params <= expr->call.args->size) {
//            add_error(errorList, "Incorrect number of arguments", expr->span, tc->source);
//            return 0;
//        }
//    } else {
//        if (var_info->type.func.num_params != expr->call.args->size) {
//            add_error(errorList, "Incorrect number of arguments", expr->span, tc->source);
//            return 0;
//        }
//    }
//    if (var_info->type.func.num_params >= expr->call.args->size) {
//        add_error(errorList, "Incorrect number of arguments", expr->span, tc->source);
//        return 0;
//    }

    for (int i = 0; i < expr->call.args->size; i++) {
        if (!check_expr(tc, expr->call.args->exprs[i])) {
            return 0;
        }
//        if (expr->call.args->exprs[i]->ty->kind != var_info->type.func.params[i].kind) {
//            add_error(errorList, "Argument type does not match function parameter type", expr->call.args->exprs[i]->span, tc->source);
//            return 0;
//        }
    }
    *expr->ty = *expr->call.name->ty->func.ret;
    return 1;
}

int init_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->init.name)) {
        return 0;
    }
    Type *type = expr->init.name->ty;
    if (type == NULL) {
        add_error(errorList, "Variable not found", expr->span, tc->source);
        return 0;
    }
    if (type->kind != TYPE_STRUCT) {
        add_error(errorList, "Expected struct", expr->span, tc->source);
        return 0;
    }
    if (type->struc.num_fields != expr->init.fields->size) {
        add_error(errorList, "Incorrect number of arguments", expr->span, tc->source);
        return 0;
    }
    for (int i = 0; i < expr->init.fields->size; i++) {
        if (!check_expr(tc, expr->init.fields->exprs[i])) {
            return 0;
        }
        if (!check_same_type(&type->struc.fields[i].ty, expr->init.fields->exprs[i]->ty)) {
            add_error(errorList, "Argument type does not match struct field type", expr->init.fields->exprs[i]->span, tc->source);
            return 0;
        }
    }
    *expr->ty = *type;
    return 1;
}

int assign_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->assign.var) || !check_expr(tc, expr->assign.val)) {
        return 0;
    }
    if (!check_same_type(expr->assign.var->ty, expr->assign.val->ty)) {
        add_error(errorList, "Types do not match", expr->span, tc->source);
        return 0;
    }
    *expr->ty = *expr->assign.val->ty;

    return 1;
}

int ref_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->ref)) {
        return 0;
    }
    *expr->ty = *new_ptr_type(expr->ref->ty);
    return 1;
}

int deref_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->deref)) {
        return 0;
    }
    if (expr->deref->ty->kind != TYPE_PTR) {
        add_error(errorList, "Expected pointer", expr->span, tc->source);
        return 0;
    }
    *expr->ty = *expr->deref->ty->inner_type;
    return 1;
}

int expr_get_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->get.expr)) {
        return 0;
    }
    Type *s = expr->get.expr->ty;
    Extension *ext = find_extension(tc->extensions, s);
    if (ext != NULL) {
        Method *method = find_method(ext->methods, expr->get.field);
        if (method != NULL) {
            Type *params = malloc(sizeof(Type) * (method->num_params+1));
            for (int i = 0; i < method->num_params; i++) {
                MethodParam param = method->args[i];
                if (param.is_self) {
                    params[i] = *expr->get.expr->ty;
                    continue;
                }
                params[i] = param.ty;
            }
            *expr->ty = *new_func_type(params, method->num_params, &method->ret, 0, method->name, 0);
            tc->expr_get_parent = expr->get.expr;
            return 1;
        }
    }
    if (expr->get.expr->ty->kind != TYPE_STRUCT) {
        add_error(errorList, "Expected struct", expr->span, tc->source);
        return 0;
    }
    for (int i = 0; i < s->struc.num_fields; i++) {
        if (compare_strings(expr->get.field, s->struc.fields[i].name) == 0) {
            *expr->ty = s->struc.fields[i].ty;
            return 1;
        }
    }
    // (a.a)("hello")
    for (int i = 0; i < s->struc.methods->size; i++) {
        if (compare_strings(expr->get.field, expr->get.expr->ty->struc.methods->methods[i]->name) == 0) {
            Type *params = malloc(sizeof(Type) * expr->get.expr->ty->struc.methods->methods[i]->num_params);
            for (int j = 0; j < expr->get.expr->ty->struc.methods->methods[i]->num_params; j++) {
                MethodParam param = expr->get.expr->ty->struc.methods->methods[i]->args[j];
                if (param.is_self) {
                    params[j] = *expr->get.expr->ty;
                    continue;
                }
                params[j] = param.ty;
            }
            *expr->ty = *new_func_type(params, s->struc.methods->methods[i]->num_params,
                                       &s->struc.methods->methods[i]->ret, 0,
                                       s->struc.methods->methods[i]->name, 0);
            tc->expr_get_parent = expr->get.expr;
            return 1;
        }
    }

    return 0;
}

int null_rule(TypeChecker *tc, Expr *expr) {
    *expr->ty = *new_type(TYPE_NULL);
    return 1;
}

int as_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->as.expr)) {
        return 0;
    }
    *expr->as.expr->ty = expr->as.ty;
    *expr->ty = expr->as.ty;
    analyze_type(tc, expr->ty);
    return 1;
}

int array_rule(TypeChecker *tc, Expr *expr) {
    Type *ty;
    int len = (int) expr->array.exprs->size;
    for (int i = 0; i < expr->array.exprs->size; i++) {
        if (!check_expr(tc, expr->array.exprs->exprs[i])) {
            return 0;
        }
        if (i == 0) {
            ty = expr->array.exprs->exprs[i]->ty;
        } else {
            if (!check_same_type(ty, expr->array.exprs->exprs[i]->ty)) {
                add_error(errorList, "Types do not match", expr->array.exprs->exprs[i]->span, tc->source);
                return 0;
            }
        }
    }
    *expr->ty = *new_array_type(ty, len);
    return 1;
}

int index_rule(TypeChecker *tc, Expr *expr) {
    if (!check_expr(tc, expr->index.list) || !check_expr(tc, expr->index.index)) {
        return 0;
    }
    if (expr->index.list->ty->kind != TYPE_ARRAY && expr->index.list->ty->kind != TYPE_STRING && expr->index.list->ty->kind != TYPE_PTR) {
        add_error(errorList, "Expected array", expr->span, tc->source);
        return 0;
    }
    if (expr->index.index->ty->kind != TYPE_INT) {
        add_error(errorList, "Expected int", expr->index.index->span, tc->source);
        return 0;
    }
    if (expr->index.list->ty->kind == TYPE_ARRAY) {
        *expr->ty = *expr->index.list->ty->array.inner_type;
    } else if (expr->index.list->ty->kind == TYPE_STRING) {
        *expr->ty = *new_type(TYPE_CHAR);
    } else {
        *expr->ty = *expr->index.list->ty->inner_type;
    }
    return 1;
}

int ns_rule(TypeChecker *tc, Expr *expr) {
    if (expr->ns.path->size == 0) {
        add_error(errorList, "Empty namespace", expr->span, tc->source);
        return 0;
    }

    Type *var_info = get_var(tc->scope_manager, expr->ns.path->strings[0]);
    if (var_info == NULL) {
        add_error(errorList, "Namespace not found", expr->span, tc->source);
        return 0;
    }

    for (int i = 1; i < expr->ns.path->size; i++) {
        if (var_info->kind != TYPE_MODULE) {
            add_error(errorList, "Expected module", expr->span, tc->source);
            return 0;
        }
        Symbol *symbol = find_symbol(var_info->module.symbols, expr->ns.path->strings[i]);
        if (symbol == NULL) {
            add_error(errorList, "Namespace not found", expr->span, tc->source);
            return 0;
        }
        symbol->type->parent = var_info;
        var_info = symbol->type;
    }

    expr->ty->parent = var_info;
    *expr->ty = *var_info;

    return 1;
}

int expr_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    return check_expr(tc, stmt->expr);
}

int if_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    if (!check_expr(tc, stmt->if_stmt.cond)) {
        return 0;
    }
    if (stmt->if_stmt.cond->ty->kind != TYPE_BOOL) {
        add_error(errorList, "Expected bool", stmt->if_stmt.cond->span, tc->source);
        return 0;
    }
    if (!check_stmt(tc, stmt->if_stmt.body)) {
        return 0;
    }
    return 1;
}

int while_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    if (!check_expr(tc, stmt->while_stmt.cond)) {
        return 0;
    }
    if (stmt->while_stmt.cond->ty->kind != TYPE_BOOL) {
        add_error(errorList, "Expected bool", stmt->while_stmt.cond->span, tc->source);
        return 0;
    }
    if (!check_stmt(tc, stmt->while_stmt.body)) {
        return 0;
    }
    return 1;
}

int for_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    if (!check_expr(tc, stmt->for_stmt.init)) {
        return 0;
    }
    if (!check_expr(tc, stmt->for_stmt.cond)) {
        return 0;
    }
    if (stmt->for_stmt.cond->ty->kind != TYPE_BOOL) {
        add_error(errorList, "Expected bool", stmt->for_stmt.cond->span, tc->source);
        return 0;
    }
    if (!check_expr(tc, stmt->for_stmt.update)) {
        return 0;
    }
    if (!check_stmt(tc, stmt->for_stmt.body)) {
        return 0;
    }
    return 1;
}

int block_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    enter_scope(tc->scope_manager);
    for (int i = 0; i < stmt->block.stmts->size; i++) {
        if (!check_stmt(tc, stmt->block.stmts->stmts[i])) {
            return 0;
        }
    }
    exit_scope(tc->scope_manager);
    return 1;
}

int return_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    if (tc->should_return.kind == TYPE_INVALID) {
        add_error(errorList, "Return statement outside of function", stmt->span, tc->source);
        return 0;
    }
    if (!check_expr(tc, stmt->ret_expr)) {
        return 0;
    }
    if (stmt->ret_expr->ty->kind != tc->should_return.kind) {
        add_error(errorList, "Return type does not match function return type", stmt->ret_expr->span, tc->source);
        return 0;
    }
    return 1;
}

int let_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    if (!check_expr(tc, stmt->let.expr)) {
        return 0;
    }
    if (stmt->let.type != NULL) {
        analyze_type(tc, stmt->let.type);
        if (!check_same_type(stmt->let.type, stmt->let.expr->ty)) {
            add_error(errorList, "Types do not match", stmt->let.expr->span, tc->source);
            return 0;
        }
    }
    add_var(tc->scope_manager, stmt->let.name, *stmt->let.expr->ty);
    return 1;
}

int func_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    Type* ret = &stmt->function.ret;
    analyze_type(tc, ret);
    Type* param = malloc(sizeof(Type) * stmt->function.num_params);
    for (int i = 0; i < stmt->function.num_params; i++) {
        analyze_type(tc, &stmt->function.args[i].ty);
        param[i] = stmt->function.args[i].ty;
    }
    add_var(tc->scope_manager, stmt->function.name, *visit_new_type(tc, new_func_type(param, stmt->function.num_params, ret, 0,
                                                                   stmt->function.name, 0)));
    enter_scope(tc->scope_manager);
    tc->current_function = stmt->function.name;
    tc->should_return = stmt->function.ret;
    for (int i = 0; i < stmt->function.num_params; i++) {
        add_var(tc->scope_manager, stmt->function.args[i].name, param[i]);
    }
    if (!check_stmt(tc, stmt->function.body)) {
        return 0;
    }
    exit_scope(tc->scope_manager);
    tc->current_function = NULL;
    return 1;
}

int struct_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    Param *fields = malloc(sizeof(Param) * stmt->struc.num_params);
    for (int i = 0; i < stmt->struc.num_params; i++) {
        analyze_type(tc, &stmt->struc.args[i].ty);
        fields[i] = stmt->struc.args[i];
    }
    Type* s = new_struct_type(stmt->struc.name, fields, stmt->struc.num_params,
                              stmt->struc.methods, new_op_overload_list());
    add_var(tc->scope_manager, stmt->struc.name, *visit_new_type(tc, s));

    for (int i = 0; i < stmt->struc.methods->size; i++) {
        Type* ret = &stmt->struc.methods->methods[i]->ret;
        analyze_type(tc, ret);
        Type* param = malloc(sizeof(Type) * stmt->struc.methods->methods[i]->num_params);
        enter_scope(tc->scope_manager);
        for (int j = 0; j < stmt->struc.methods->methods[i]->num_params; j++) {
            if (stmt->struc.methods->methods[i]->args[j].is_self) {
                stmt->struc.methods->methods[i]->args[j].ty = *s;
                add_var(tc->scope_manager, new_string("self"), stmt->struc.methods->methods[i]->args[j].ty);
                continue;
            }
            analyze_type(tc, &stmt->struc.methods->methods[i]->args[j].ty);
            param[j] = stmt->struc.methods->methods[i]->args[j].ty;
            add_var(tc->scope_manager, stmt->struc.methods->methods[i]->args[j].name, stmt->struc.methods->methods[i]->args[j].ty);
        }
        tc->current_function = stmt->struc.methods->methods[i]->name;
        tc->current_struct = stmt->struc.name;
        tc->should_return = stmt->struc.methods->methods[i]->ret;
        if (!check_stmt(tc, stmt->struc.methods->methods[i]->body)) {
            return 0;
        }
        exit_scope(tc->scope_manager);
        tc->current_struct = NULL;

    }

    for (int i = 0; i < stmt->struc.overloads->size; i++) {
        OpOverload *overload = stmt->struc.overloads->overloads[i];
        Type *ret = &overload->method->ret;
        analyze_type(tc, ret);
        enter_scope(tc->scope_manager);
        for (int j = 0; j < overload->method->num_params; j++) {
            MethodParam param = overload->method->args[j];
            if (param.is_self) {
                analyze_type(tc, s);
                overload->method->args[j].ty = *s;
                add_var(tc->scope_manager, new_string("self"), *s);
                continue;
            }
            analyze_type(tc, &overload->method->args[j].ty);
            analyze_type(tc, &param.ty);
            add_var(tc->scope_manager, param.name, param.ty);
        }
        tc->current_function = overload->method->name;
        tc->current_struct = stmt->struc.name;
        tc->should_return = overload->method->ret;
        if (!check_stmt(tc, overload->method->body)) {
            return 0;
        }
        exit_scope(tc->scope_manager);
        tc->current_struct = NULL;
        append_op_overload(s->struc.op_overloads, overload);

    }
    return 1;
}

int extern_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    Type* param = malloc(sizeof(Type) * stmt->extern_.num_params);
    for (int i = 0; i < stmt->extern_.num_params; i++) {
        analyze_type(tc, &stmt->extern_.args[i].ty);
        param[i] = stmt->extern_.args[i].ty;
    }
    add_var(tc->scope_manager, stmt->extern_.name, *visit_new_type(tc, new_func_type(param, stmt->extern_.num_params,
                                                                  &stmt->extern_.ret, stmt->extern_.is_vararg,
                                                                  stmt->extern_.name, 1)));
    return 1;
}

int import_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    for (int i = 0; i < tc->module_paths->size; i++) {
        String *path = new_string(tc->module_paths->strings[i]->data);
        add_string(path, stmt->import.name);
        add_c_string(path, ".jack");
        FILE *file = fopen(path->data, "r");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long length = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *buffer = malloc(length + 1);
            if (buffer == NULL) {
                fclose(file);
                return 0;
            }
            fread(buffer, 1, length, file);

            StmtList *stmts = parse(strdup(buffer), path->data);
            Symbols *symbols = get_symbols_from_stmts(stmts);
            TypeChecker *new_tc = new_type_checker(new_symbols(), stmts, buffer);
            Type *s = new_module_type(symbols->symbols, stmt->import.name);
            new_tc->module_paths = tc->module_paths;
            *new_tc->parent_type = *s;
            new_tc->extensions = tc->extensions;
            for (int j = 0; j < stmts->size; j++) {
                if (!check_stmt(new_tc, stmts->stmts[j])) {
                    return 0;
                }
            }
            tc->extensions = new_tc->extensions;
            Stmt* module_stmt = new_module_stmt(stmt->import.name, stmts, stmt->span);
            replace_stmts(tc->stmts, stmt, module_stmt);
            Symbol *symbol = new_symbol(stmt->import.name, s, 0, NULL);
            if (!append_symbol(tc->symbols->symbols, symbol)) {
                return 0;
            }
            add_var_info(tc->scope_manager->global, new_var_info(stmt->import.name, *new_module_type(symbols->symbols,
                                                                                                     stmt->import.name)));
            return 1;
        }
    }
    add_error(errorList, "Module not found", stmt->span, tc->source);
    return 0;
}

int module_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    Symbols *symbols = get_symbols_from_stmts(stmt->module.stmts);
    Type* parent = new_module_type(symbols->symbols, stmt->module.name);
    add_var(tc->scope_manager, stmt->module.name, *parent);
    tc->module_paths = tc->module_paths;
    Type *old_parent = tc->parent_type;
    tc->parent_type = parent;
    for (int i = 0; i < stmt->module.stmts->size; i++) {
        if (!check_stmt(tc, stmt->module.stmts->stmts[i])) {
            return 0;
        }
    }

    tc->parent_type = old_parent;


    return 1;
}

int extension_stmt_rule(TypeChecker *tc, Stmt *stmt) {
    analyze_type(tc, &stmt->extension->ty);
    Type *s = &stmt->extension->ty;
    for (int i = 0; i < stmt->extension->methods->size; i++) {
        Type* ret = &stmt->extension->methods->methods[i]->ret;
        analyze_type(tc, ret);
        Type* param = malloc(sizeof(Type) * stmt->extension->methods->methods[i]->num_params);
        enter_scope(tc->scope_manager);
        for (int j = 0; j < stmt->extension->methods->methods[i]->num_params; j++) {
            if (stmt->extension->methods->methods[i]->args[j].is_self) {
                stmt->extension->methods->methods[i]->args[j].ty = *s;
                add_var(tc->scope_manager, new_string("self"), stmt->extension->methods->methods[i]->args[j].ty);
                continue;
            }
            analyze_type(tc, &stmt->extension->methods->methods[i]->args[j].ty);
            param[j] = stmt->extension->methods->methods[i]->args[j].ty;
            add_var(tc->scope_manager, stmt->extension->methods->methods[i]->args[j].name, stmt->extension->methods->methods[i]->args[j].ty);
        }
        tc->should_return = stmt->extension->methods->methods[i]->ret;
        if (!check_stmt(tc, stmt->extension->methods->methods[i]->body)) {
            return 0;
        }
        exit_scope(tc->scope_manager);
    }
    Extension *ext = find_extension(tc->extensions, &stmt->extension->ty);
    if (ext != NULL) {
        for (int i = 0; i < stmt->extension->methods->size; i++) {
            append_method(ext->methods, stmt->extension->methods->methods[i]);
        }
        return 1;
    }
    append_extension(tc->extensions, stmt->extension);
    return 1;
}

int append_expr_rule(ExprRules *rules, ExprRule *rule) {
    if (rules->size == rules->capacity) {
        rules->capacity *= 2;
        ExprRule** new_rules = malloc(sizeof(ExprRule*) * rules->capacity);
        if (new_rules == NULL) {
            return -1;
        }
        memcpy(new_rules, rules->rules, sizeof(ExprRule*) * rules->size);
        rules->rules = new_rules;
    }
    rules->rules[rules->size++] = rule;
    return 1;

}

ExprRules *new_expr_rules() {
    ExprRules *rules = malloc(sizeof(ExprRules));
    if (rules == NULL) {
        return NULL;
    }
    rules->rules = malloc(sizeof(ExprRule) * 10);
    if (rules->rules == NULL) {
        free(rules);
        return NULL;
    }
    rules->size = 0;
    rules->capacity = 10;
    return rules;
}

ExprRule *new_expr_rule(ExprType type, ExprRuleFunc *rule) {
    ExprRule *r = malloc(sizeof(ExprRule));
    if (r == NULL) {
        return NULL;
    }
    r->type = type;
    r->rule = rule;
    return r;
}

void free_expr_rules(ExprRules *rules) {
    free(rules->rules);
    free(rules);
}

StmtRules *new_stmt_rules() {
    StmtRules *rules = malloc(sizeof(StmtRules));
    if (rules == NULL) {
        return NULL;
    }
    rules->rules = malloc(sizeof(StmtRule) * 10);
    if (rules->rules == NULL) {
        free(rules);
        return NULL;
    }
    rules->size = 0;
    rules->capacity = 10;
    return rules;
}

int append_stmt_rule(StmtRules *rules, StmtRule *rule) {
    if (rules->size == rules->capacity) {
        rules->capacity *= 2;
        StmtRule **new_rules = malloc(sizeof(StmtRule*) * rules->capacity);
        if (new_rules == NULL) {
            return -1;
        }
        memcpy(new_rules, rules->rules, sizeof(StmtRule*) * rules->size);
        rules->rules = new_rules;
    }
    rules->rules[rules->size++] = rule;
    return 1;
}

StmtRule *new_stmt_rule(StmtType type, StmtRuleFunc *rule) {
    StmtRule *r = malloc(sizeof(StmtRule));
    if (r == NULL) {
        return NULL;
    }
    r->type = type;
    r->rule = rule;
    return r;
}

VarInfo *new_var_info(String *name, Type type) {
    VarInfo *vi = malloc(sizeof(VarInfo));
    if (vi == NULL) {
        return NULL;
    }
    vi->name = name;
    vi->type = type;
    return vi;
}


VarInfoList *new_var_info_list() {
    VarInfoList *vil = malloc(sizeof(VarInfoList));
    if (vil == NULL) {
        return NULL;
    }
    vil->vars = malloc(sizeof(VarInfo) * 10);
    if (vil->vars == NULL) {
        free(vil);
        return NULL;
    }
    vil->size = 0;
    vil->capacity = 10;
    return vil;
}

int append_var_info(VarInfoList *list, VarInfo *var_info) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->vars = realloc(list->vars, sizeof(VarInfo) * list->capacity);
        if (list->vars == NULL) {
            return -1;
        }
    }
    list->vars[list->size++] = var_info;
    return 1;
}

void free_var_info_list(VarInfoList *list) {
    free(list->vars);
    free(list);
}

Scope *new_scope(Scope *parent) {
    Scope *s = malloc(sizeof(Scope));
    if (s == NULL) {
        return NULL;
    }
    s->parent = (struct Scope *) parent;
    s->scope = new_var_info_list();
    if (s->scope == NULL) {
        free(s);
        return NULL;
    }
    return s;
}

VarInfo *get_var_info(Scope *scope, String *name) {
    for (int i = 0; i < scope->scope->size; i++) {
        if (compare_string(scope->scope->vars[i]->name, name->data) == 0) {
            return scope->scope->vars[i];
        }
    }
    if (scope->parent == NULL) {
        return NULL;
    }
    return get_var_info((Scope *) scope->parent, name);
}

int add_var_info(Scope *scope, VarInfo *var_info) {
    return append_var_info(scope->scope, var_info);
}

void free_scope(Scope *scope) {
    free_var_info_list(scope->scope);
    free(scope);
}

ScopeManager *new_scope_manager() {
    ScopeManager *sm = malloc(sizeof(ScopeManager));
    if (sm == NULL) {
        return NULL;
    }
    sm->global = new_scope(NULL);
    if (sm->global == NULL) {
        free(sm);
        return NULL;
    }
    sm->current = sm->global;
    return sm;
}
Type* get_var(ScopeManager *manager, String *name) {
    VarInfo *var_info = get_var_info(manager->current, name);
    if (var_info == NULL) {
        return NULL;
    }
    return &var_info->type;
}

int add_var(ScopeManager *manager, String *name, Type type) {
    VarInfo *var_info = new_var_info(name, type);
    if (var_info == NULL) {
        return -1;
    }
    return add_var_info(manager->current, var_info);
}

int enter_scope(ScopeManager *manager) {
    Scope *s = new_scope(manager->current);
    if (s == NULL) {
        return -1;
    }
    manager->current = s;
    return 1;
}

int exit_scope(ScopeManager *manager) {
    if (manager->current->parent == NULL) {
        return -1;
    }
    Scope *parent = (Scope *) manager->current->parent;
    free_scope(manager->current);
    manager->current = parent;
    return 1;
}

TypeChecker *new_type_checker(Symbols *symbols, StmtList *stmts, const char* source) {
    TypeChecker *tc = malloc(sizeof(TypeChecker));
    if (tc == NULL) {
        return NULL;
    }
    tc->stmts = stmts;
    tc->should_return = *new_type(TYPE_INVALID);
    tc->expr_get_parent = malloc(sizeof(Expr));
    if (tc->expr_get_parent == NULL) {
        free(tc);
        return NULL;
    }
    tc->expr_rules = new_expr_rules();
    if (tc->expr_rules == NULL) {
        free(tc);
        return NULL;
    }
    tc->stmt_rules = new_stmt_rules();
    if (tc->stmt_rules == NULL) {
        free(tc->expr_rules);
        free(tc);
        return NULL;
    }
    tc->scope_manager = new_scope_manager();
    if (tc->scope_manager == NULL) {
        free(tc->stmt_rules);
        free(tc->expr_rules);
        free(tc);
        return NULL;
    }
    tc->symbols = symbols;
    tc->extensions = new_extension_list();
    tc->parent_type = new_type(TYPE_INVALID);
    tc->module_paths = new_string_list();
    append_string(tc->module_paths, new_string("/Users/antoine/.jack/"));

    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_INT, int_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_FLOAT, float_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_STRING, string_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_CHAR, char_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_IDENT, ident_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_BINOP, binop_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_CALL, call_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_INIT, init_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_GET, expr_get_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_ASSIGN, assign_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_REF, ref_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_DEREF, deref_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_NULL, null_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_AS, as_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_ARRAY, array_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_INDEX, index_rule));
    append_expr_rule(tc->expr_rules, new_expr_rule(EXPR_NS, ns_rule));

    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_EXPR, expr_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_IF, if_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_WHILE, while_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_FOR, for_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_BLOCK, block_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_RETURN, return_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_LET, let_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_FUNCTION, func_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_STRUCT, struct_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_EXTERN, extern_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_IMPORT, import_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_MODULE, module_stmt_rule));
    append_stmt_rule(tc->stmt_rules, new_stmt_rule(STMT_EXTENSION, extension_stmt_rule));

    tc->source = new_string(source);
    return tc;
}


int check_expr(TypeChecker *tc, Expr *expr) {
    for (int i = 0; i < tc->expr_rules->size; i++) {
        if (tc->expr_rules->rules[i]->type == expr->type && tc->expr_rules->rules[i]->rule(tc, expr)) {
            return 1;
        }
    }
    return 0;
}

int check_stmt(TypeChecker *tc, Stmt *stmt) {
    for (int i = 0; i < tc->stmt_rules->size; i++) {
        if (tc->stmt_rules->rules[i]->type == stmt->type && tc->stmt_rules->rules[i]->rule(tc, stmt)) {
            return 1;
        }
    }
    return 0;
}

int check_tc(TypeChecker *tc) {
    for (int k = 0; k < tc->symbols->symbols->size; k++) {
        add_var(tc->scope_manager, tc->symbols->symbols->symbols[k]->name, *tc->symbols->symbols->symbols[k]->type);
    }
    int n = 0;
    for (int i = 0; i < tc->stmts->size; i++) {
        if (check_stmt(tc,  tc->stmts->stmts[i])) {
            n++;
        }
    }
    return n == tc->stmts->size;
}


