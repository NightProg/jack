#include "llvm_codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <llvm-c/ExecutionEngine.h>
#include "mangler.h"
#include "ast.h"
#include "parser.h"

LLVMStructInfo *new_llvm_struct_info(String *name, LLVMTypeRef type, Param *params) {
    LLVMStructInfo *info = malloc(sizeof(LLVMStructInfo));
    if (info == NULL) {
        return NULL;
    }
    info->name = name;
    info->type = type;
    info->params = params;
    return info;
}


void free_llvm_struct_info(LLVMStructInfo *info) {
    free(info);
}

LLVMStructInfoList *new_llvm_struct_info_list() {
    LLVMStructInfoList *list = malloc(sizeof(LLVMStructInfoList));
    if (list == NULL) {
        return NULL;
    }
    list->structs = malloc(sizeof(LLVMStructInfo*) * 4);
    if (list->structs == NULL) {
        free(list);
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    return list;
}


int append_llvm_struct_info(LLVMStructInfoList *list, String *name, LLVMTypeRef type, Param *params) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        LLVMStructInfo **new_structs = malloc(sizeof(LLVMStructInfo*) * list->capacity);
        if (new_structs == NULL) {
            return -1;
        }
        memcpy(new_structs, list->structs, sizeof(LLVMStructInfo*) * list->size);
        free(list->structs);
        list->structs = new_structs;
    }
    list->structs[list->size++] = new_llvm_struct_info(name, type, params);
    return 0;
}



LLVMStructInfo *get_llvm_struct_info(LLVMStructInfoList *list, String *name) {
    for (int i = 0; i < list->size; i++) {
        if (strcmp(list->structs[i]->name->data, name->data) == 0) {
            return list->structs[i];
        }
    }
    return NULL;
}

void free_llvm_struct_info_list(LLVMStructInfoList *list) {
    for (int i = 0; i < list->size; i++) {
        free_llvm_struct_info(list->structs[i]);
    }
    free(list->structs);
    free(list);
}

LLVMVarInfo *new_llvm_var_info(String *name, LLVMValueRef value, LLVMTypeRef type) {
    LLVMVarInfo *info = malloc(sizeof(LLVMVarInfo));
    if (info == NULL) {
        return NULL;
    }
    info->name = name;
    info->value = value;
    info->type = type;
    return info;
}

void free_llvm_var_info(LLVMVarInfo *info) {
    free(info);
}

LLVMVarInfoList *new_llvm_var_info_list() {
    LLVMVarInfoList *list = malloc(sizeof(LLVMVarInfoList));
    if (list == NULL) {
        return NULL;
    }
    list->vars = malloc(sizeof(LLVMVarInfo*) * 4);
    if (list->vars == NULL) {
        free(list);
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    return list;
}

int clear_llvm_var_info(LLVMVarInfoList *list) {
    int n = list->size;
    for (int i = 0; i < list->size; i++) {
        if (LLVMGetTypeKind(list->vars[i]->type) == LLVMFunctionTypeKind) {
            continue;
        }
        n--;
        free_llvm_var_info(list->vars[i]);
    }
    list->size = n;
    for (int i = 0; i < list->size; i++) {
        list->vars[i] = list->vars[i + n];
    }
    return 0;
}

LLVMVarInfoList *filter_llvm_var_info(LLVMVarInfoList *list) {
    LLVMVarInfoList *new_list = new_llvm_var_info_list();
    for (int i = 0; i < list->size; i++) {
        if (LLVMGetTypeKind(list->vars[i]->type) != LLVMFunctionTypeKind) {
            continue;
        }
        append_llvm_var_info(new_list, list->vars[i]->name, list->vars[i]->value, list->vars[i]->type);
    }
    return new_list;
}

int append_llvm_var_info(LLVMVarInfoList *list, String *name, LLVMValueRef value, LLVMTypeRef type) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        LLVMVarInfo **new_vars = malloc(sizeof(LLVMVarInfo*) * list->capacity);
        if (new_vars == NULL) {
            return -1;
        }
        memcpy(new_vars, list->vars, sizeof(LLVMVarInfo*) * list->size);
        free(list->vars);
        list->vars = new_vars;
    }
    list->vars[list->size++] = new_llvm_var_info(name, value, type);
    return 0;
}

LLVMVarInfo *get_llvm_var_info(LLVMVarInfoList *list, String *name) {
    for (int i = 0; i < list->size; i++) {
        if (strcmp(list->vars[i]->name->data, name->data) == 0) {
            return list->vars[i];
        }
    }
    return NULL;
}

void free_llvm_var_info_list(LLVMVarInfoList *list) {
    for (int i = 0; i < list->size; i++) {
        free_llvm_var_info(list->vars[i]);
    }
    free(list->vars);
    free(list);
}

LLVMCodeGen *new_llvm_codegen(StmtList *stmts, Symbols *symbols, const char *module_name) {
    LLVMCodeGen *codegen = malloc(sizeof(LLVMCodeGen));
    if (codegen == NULL) {
        return NULL;
    }
    codegen->context = LLVMContextCreate();
    codegen->module = LLVMModuleCreateWithNameInContext(module_name, codegen->context);
    codegen->builder = LLVMCreateBuilderInContext(codegen->context);
    codegen->symbols = symbols;
    codegen->current_function = NULL;
    codegen->vars = new_llvm_var_info_list();
    codegen->stmts = stmts;
    codegen->structs = new_llvm_struct_info_list();
    codegen->current_module = new_string("");
    return codegen;
}


LLVMTypeRef llvm_type(LLVMCodeGen *codegen, Type type) {
    switch (type.kind) {
        case TYPE_INT:
            return LLVMInt32TypeInContext(codegen->context);
        case TYPE_BOOL:
            return LLVMInt1TypeInContext(codegen->context);
        case TYPE_CHAR:
            return LLVMInt8TypeInContext(codegen->context);
        case TYPE_STRING:
            return LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0);
        case TYPE_STRUCT: {
            if (compare_string(codegen->current_module, "") != 0) {
                String *mangled = mangle_module(codegen->current_module, type.struc.name);
                LLVMStructInfo *info = get_llvm_struct_info(codegen->structs, mangled);
                if (info == NULL) {
                    printf("Unknown struct 0 \n");
                    return NULL;
                }
                return LLVMPointerType(info->type, 0);
            }
            if (type.parent != NULL) {
                if (type.parent->kind != TYPE_MODULE) {
                    printf("Unknown parent\n");
                    return NULL;
                }
                String *mangled = mangle_module(type.parent->module.name, type.struc.name);
                LLVMStructInfo *info = get_llvm_struct_info(codegen->structs, mangled);
                if (info == NULL) {
                    printf("Unknown struct 2\n");
                    return NULL;
                }
                return LLVMPointerType(info->type, 0);
            }
            LLVMStructInfo *info = get_llvm_struct_info(codegen->structs, type.struc.name);
            if (info == NULL) {
                printf("Unknown struct 1\n");
                return NULL;
            }
            return LLVMPointerType(info->type, 0);
        }
        case TYPE_PTR:
            return LLVMPointerType(llvm_type(codegen, *type.inner_type), 0);
        case TYPE_VOID:
            return LLVMVoidTypeInContext(codegen->context);
        case TYPE_ARRAY: {
            return LLVMArrayType(llvm_type(codegen, *type.array.inner_type), type.array.length);
        }
        case TYPE_FUNC: {
            LLVMTypeRef *param_types = malloc(sizeof(LLVMTypeRef) * type.func.num_params);
            for (int i = 0; i < type.func.num_params; i++) {
                param_types[i] = llvm_type(codegen, type.func.params[i]);
            }
            return LLVMFunctionType(llvm_type(codegen, *type.func.ret), param_types, type.func.num_params, type.func.is_vararg);
        }
        default:
            printf("Unknown type\n");
            return NULL;
    }
}

LLVMValueRef llvm_codegen_expr(LLVMCodeGen *codegen, Expr *expr) {
    switch (expr->type) {
        case EXPR_INT: {
            return LLVMConstInt(llvm_type(codegen, *new_type(TYPE_INT)), expr->int_val, 0);
        }
        case EXPR_CHAR: {
            return LLVMConstInt(llvm_type(codegen, *new_type(TYPE_CHAR)), expr->char_val, 0);
        }
        case EXPR_STRING: {
            String* s = fake_ws_to_real_ws(expr->string_val);
            return LLVMBuildGlobalStringPtr(codegen->builder, s->data, "strtmp");
        }
        case EXPR_IDENT:
            for (int i = 0; i < codegen->vars->size; i++) {
                if (strcmp(codegen->vars->vars[i]->name->data, expr->ident_val->data) == 0) {
                    if (LLVMGetTypeKind(codegen->vars->vars[i]->type) == LLVMFunctionTypeKind) {
                        return codegen->vars->vars[i]->value;
                    }
//                    return codegen->vars->vars[i]->value;
                    if (LLVMIsAArgument(codegen->vars->vars[i]->value) || codegen->no_load) {
                        return codegen->vars->vars[i]->value;
                    }
                    if (expr->ty->kind == TYPE_STRUCT) {
                        return codegen->vars->vars[i]->value;
                    }
                    return LLVMBuildLoad2(codegen->builder, codegen->vars->vars[i]->type, codegen->vars->vars[i]->value, "loadtmp");
                }
            }
            printf("Unknown variable\n");
            return NULL;
        case EXPR_AS: {
            return llvm_codegen_expr(codegen, expr->as.expr);
        }
        case EXPR_ARRAY: {
            LLVMValueRef *values = malloc(sizeof(LLVMValueRef) * expr->array.exprs->size);

            for (int i = 0; i < expr->array.exprs->size; i++) {
                values[i] = llvm_codegen_expr(codegen, expr->array.exprs->exprs[i]);
            }

            LLVMValueRef alloca = LLVMBuildAlloca(codegen->builder, llvm_type(codegen, *expr->ty), "arraytmp");
            for (int i = 0; i < expr->array.exprs->size; i++) {
                LLVMValueRef index = LLVMConstInt(LLVMInt32TypeInContext(codegen->context), i, 0);
                LLVMValueRef ptr = LLVMBuildGEP2(codegen->builder, llvm_type(codegen, *expr->array.exprs->exprs[i]->ty), alloca, &index, 1, "geptmp");
                LLVMBuildStore(codegen->builder, values[i], ptr);
            }
            return alloca;
        }

        case EXPR_INIT: {
            LLVMStructInfo *info;
            if (expr->init.name->type == EXPR_IDENT) {
                info = get_llvm_struct_info(codegen->structs, expr->init.name->ident_val);
                if (info == NULL) {
                    printf("Unknown struct 4\n");
                    return NULL;
                }
        
            } else if (expr->init.name->type == EXPR_NS) {
                String *mangled  = mangle_namespace(expr->init.name->ns.path);
                info = get_llvm_struct_info(codegen->structs, mangled);
                if (info == NULL) {
                    printf("Unknown struct 5\n");
                    return NULL;
                }
            } else {
                printf("Unknown struct 6\n");
                return NULL;
            }
            LLVMTypeRef struct_type = info->type;
            LLVMValueRef struct_value = LLVMBuildAlloca(codegen->builder, struct_type, "structtmp");
            for (int i = 0; i < expr->init.fields->size; i++) {
                LLVMValueRef field = llvm_codegen_expr(codegen, expr->init.fields->exprs[i]);
                LLVMBuildStore(codegen->builder, field, LLVMBuildStructGEP2(codegen->builder, struct_type,  struct_value, i, "fieldtmp"));
            }
            if (codegen->should_be_i64) {
                return LLVMBuildLoad2(codegen->builder, LLVMInt64TypeInContext(codegen->context), struct_value, "loadtmp");
            }
            return struct_value;
        }
        case EXPR_GET: {
            codegen->no_load++;
            String *name;
            if (expr->get.expr->ty->parent != NULL) {
                name = mangle_module(expr->get.expr->ty->parent->module.name, expr->get.expr->ty->struc.name);
            } else {
                name = expr->get.expr->ty->struc.name;
            }
            LLVMStructInfo *info = get_llvm_struct_info(codegen->structs, name);

            if (info == NULL) {
                printf("Unknown struct 8\n");
                return NULL;
            }
            LLVMTypeRef struct_type = info->type;
            LLVMValueRef struct_value = llvm_codegen_expr(codegen, expr->get.expr);
            codegen->no_load--;
            unsigned int length = LLVMCountStructElementTypes(struct_type);
            for (int i = 0; i < length; i++) {
                if (strcmp(info->params[i].name->data, expr->get.field->data) == 0) {
                    if (codegen->no_load) {
                        return LLVMBuildStructGEP2(codegen->builder, struct_type, struct_value, i, "fieldtmp");
                    }
                    return LLVMBuildLoad2(codegen->builder, LLVMStructGetTypeAtIndex(struct_type, i), LLVMBuildStructGEP2(codegen->builder, struct_type, struct_value, i, "fieldtmp"), "loadtmp");
                }
            }

            String *mangled = mangle_struct_method(name, expr->get.field);
            LLVMValueRef func = get_llvm_var_info(codegen->vars, mangled)->value;
            if (func == NULL) {
                printf("Unknown method\n");
                return NULL;
            }
            return func;
        }
        case EXPR_BINOP: {
            LLVMValueRef left = llvm_codegen_expr(codegen, expr->binop.lhs);

            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMPointerTypeKind && expr->binop.op == BINOP_ADD) {
                int old_load = codegen->no_load;
                codegen->no_load = 0;
                LLVMValueRef lc = llvm_codegen_expr(codegen, expr->binop.lhs);
                LLVMValueRef rc = llvm_codegen_expr(codegen, expr->binop.rhs);
                codegen->no_load = old_load;
//                LLVMValueRef l = LLVMBuildLoad2(codegen->builder, LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0), lc, "loadtmp");
                return LLVMBuildGEP2(codegen->builder, llvm_type(codegen, *expr->binop.lhs->ty->inner_type), lc, &rc, 1, "geptmp");
            }

            LLVMValueRef right = llvm_codegen_expr(codegen, expr->binop.rhs);

            switch (expr->binop.op) {
                case BINOP_ADD: {
                    return LLVMBuildAdd(codegen->builder, left, right, "addtmp");
                }
                case BINOP_SUB: {
                    return LLVMBuildSub(codegen->builder, left, right, "subtmp");
                }
                case BINOP_MUL: {
                    return LLVMBuildMul(codegen->builder, left, right, "multmp");
                }
                case BINOP_DIV: {
                    return LLVMBuildSDiv(codegen->builder, left, right, "divtmp");
                }
                case BINOP_EQ: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntEQ, left, right, "eqtmp");
                }
                case BINOP_NEQ: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntNE, left, right, "neqtmp");
                }
                case BINOP_LT: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntSLT, left, right, "lttmp");
                }
                case BINOP_GT: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntSGT, left, right, "gttmp");
                }
                case BINOP_LTE: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntSLE, left, right, "ltetmp");
                }
                case BINOP_GTE: {
                    return LLVMBuildICmp(codegen->builder, LLVMIntSGE, left, right, "gtetmp");
                }
                default:
                    printf("Unknown binary operator\n");
                    return NULL;
            }
        }
        case EXPR_NULL:
            return LLVMConstNull(LLVMPointerType(LLVMInt8TypeInContext(codegen->context), 0));
        case EXPR_CALL: {
            LLVMValueRef func = llvm_codegen_expr(codegen, expr->call.name);
            LLVMTypeRef func_type = llvm_type(codegen, *expr->call.name->ty);

            LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * expr->call.args->size);
            for (int i = 0; i < expr->call.args->size; i++) {
//                if (expr->call.args->exprs[i]->ty->kind == TYPE_STRUCT) {
//                    args[i] = LLVMBuildLoad2(codegen->builder, LLVMInt64TypeInContext(codegen->context), llvm_codegen_expr(codegen, expr->call.args->exprs[i]), "structtmp");
//                    continue;
//                }
                args[i] = llvm_codegen_expr(codegen, expr->call.args->exprs[i]);
            }
            return LLVMBuildCall2(codegen->builder, func_type, func, args, expr->call.args->size, "calltmp");
        }
        case EXPR_REF: {
//            if (expr->ref->ty->kind == TYPE_STRUCT) {
//                LLVMStructInfo *info = get_llvm_struct_info(codegen->structs, expr->ref->ty->struc.name);
//                if (info == NULL) {
//                    printf("Unknown struct\n");
//                    return NULL;
//                }
//                LLVMTypeRef struct_type = info->type;
//                LLVMValueRef struct_value = LLVMBuildAlloca(codegen->builder, struct_type, "structtmp");
////                codegen->no_load++;
//                LLVMValueRef value = llvm_codegen_expr(codegen, expr->ref);
////                codegen->no_load--;
//                LLVMBuildStore(codegen->builder, value, struct_value);
//                return struct_value;
//            }
            LLVMValueRef allocatmp = LLVMBuildAlloca(codegen->builder, LLVMPointerType(llvm_type(codegen,
                                                                                                 *expr->ref->ty), 0), "allocatmp");
            codegen->no_load++;
            LLVMValueRef value = llvm_codegen_expr(codegen, expr->ref);
            codegen->no_load--;
            LLVMBuildStore(codegen->builder, value, allocatmp);
            return allocatmp;
        }
        case EXPR_DEREF: {
            LLVMValueRef value = llvm_codegen_expr(codegen, expr->deref);
            if (expr->deref->type == EXPR_BINOP) {
                return value;
            }
            return LLVMBuildLoad2(codegen->builder, llvm_type(codegen, *expr->deref->ty), value, "dereftmp");
        }
        case EXPR_ASSIGN: {
            codegen->no_load++;
            LLVMValueRef value = llvm_codegen_expr(codegen, expr->assign.var);
            codegen->no_load--;
            codegen->should_be_i64++;
            LLVMValueRef var = llvm_codegen_expr(codegen, expr->assign.val);
            codegen->should_be_i64--;
            LLVMBuildStore(codegen->builder, var, value);
            break;
        }
        case EXPR_INDEX: {
            LLVMValueRef list = llvm_codegen_expr(codegen, expr->index.list);
            codegen->no_load++;
            LLVMValueRef index = llvm_codegen_expr(codegen, expr->index.index);
            codegen->no_load--;
            LLVMValueRef ptr = LLVMBuildGEP2(codegen->builder, llvm_type(codegen, *expr->index.list->ty->inner_type),
                                             list, &index, 1, "geptmp");
            return LLVMBuildLoad2(codegen->builder, llvm_type(codegen, *expr->index.list->ty->inner_type), ptr,
                                  "loadtmp");
        }
        case EXPR_NS: {
            if (expr->ty->kind == TYPE_FUNC) {
                String *name;
                if (expr->ty->func.is_extern) {
                    name = expr->ty->func.name;
                } else {
                    name = mangle_namespace(expr->ns.path);
                }
                LLVMValueRef func = get_llvm_var_info(codegen->vars, name)->value;
                if (func == NULL) {
                    printf("Unknown function\n");
                    return NULL;
                }
                return func;
            } else {
                printf("Unknown namespace\n");
            }
        }
        default:
            return NULL;
    }
}

void llvm_codegen_function(LLVMCodeGen *codegen, LLVMTypeRef func_type, LLVMValueRef func, Stmt* stmt) {
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMPositionBuilderAtEnd(codegen->builder, entry);
    for (int i = 0; i < stmt->function.num_params; i++) {
        LLVMValueRef arg = LLVMGetParam(func, i);
        LLVMSetValueName2(arg, stmt->function.args[i].name->data, strlen(stmt->function.args[i].name->data));
        if (stmt->function.args[i].ty.kind == TYPE_STRUCT) {
            LLVMTypeRef t = get_llvm_struct_info(codegen->structs, stmt->function.args[i].ty.struc.name)->type;
            LLVMValueRef alloca = LLVMBuildAlloca(codegen->builder, t, "allocatmp");
            LLVMBuildStore(codegen->builder, arg, alloca);
            append_llvm_var_info(codegen->vars, stmt->function.args[i].name, alloca, llvm_type(codegen,
                                                                                                stmt->function.args[i].ty));
            continue;
        }
        append_llvm_var_info(codegen->vars, stmt->function.args[i].name, arg, LLVMTypeOf(arg));
    }
    codegen->current_function = func;
    append_llvm_var_info(codegen->vars, stmt->function.name, func, func_type);
    llvm_codegen_stmt(codegen, stmt->function.body);
    codegen->vars = filter_llvm_var_info(codegen->vars);
}

void llvm_codegen_stmt(LLVMCodeGen *codegen, Stmt *stmt) {
    switch (stmt->type) {
        case STMT_EXPR: {
            llvm_codegen_expr(codegen, stmt->expr);
            break;
        }
        case STMT_LET: {
            LLVMValueRef value = llvm_codegen_expr(codegen, stmt->let.expr);
            if (stmt->let.expr->ty->kind == TYPE_STRUCT) {
                append_llvm_var_info(codegen->vars, stmt->let.name, value, llvm_type(codegen, *stmt->let.expr->ty));
                break;
            }
            LLVMValueRef alloca = LLVMBuildAlloca(codegen->builder, LLVMTypeOf(value), stmt->let.name->data);
            LLVMBuildStore(codegen->builder, value, alloca);
            append_llvm_var_info(codegen->vars, stmt->let.name, alloca, LLVMTypeOf(value));
//            if (stmt->let.expr->type == EXPR_INIT) {
//                append_llvm_var_info(codegen->vars, stmt->let.name, value, LLVMInt64TypeInContext(codegen->context));
//                break;
//            }
//            append_llvm_var_info(codegen->vars, stmt->let.name, value, llvm_type(codegen, *stmt->let.expr->ty));
            break;
        }
        case STMT_BLOCK:
            for (int i = 0; i < stmt->block.stmts->size; i++) {
                llvm_codegen_stmt(codegen, stmt->block.stmts->stmts[i]);
            }
            break;
        case STMT_RETURN:
            LLVMBuildRet(codegen->builder, llvm_codegen_expr(codegen, stmt->ret_expr));
            break;
        case STMT_FUNCTION: {
                LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * stmt->function.num_params);
                for (int i = 0; i < stmt->function.num_params; i++) {
                    param_types[i] = llvm_type(codegen, stmt->function.args[i].ty);
                }
                LLVMTypeRef func_type = LLVMFunctionType(llvm_type(codegen, stmt->function.ret), param_types, stmt->function.num_params, 0);

                LLVMValueRef func = LLVMAddFunction(codegen->module, stmt->function.name->data, func_type);
                llvm_codegen_function(codegen, func_type, func, stmt);
            break;
        }
        case STMT_EXTERN: {
            LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * stmt->extern_.num_params);
            for (int i = 0; i < stmt->extern_.num_params; i++) {
                param_types[i] = llvm_type(codegen, stmt->extern_.args[i].ty);
            }
            LLVMTypeRef func_type = LLVMFunctionType(llvm_type(codegen, stmt->extern_.ret), param_types, stmt->function.num_params, stmt->extern_.is_vararg);
            LLVMValueRef func = LLVMAddFunction(codegen->module, stmt->function.name->data, func_type);
            append_llvm_var_info(codegen->vars, stmt->function.name, func, func_type);
            break;
        }

        case STMT_STRUCT: {
            LLVMTypeRef r = LLVMStructCreateNamed(codegen->context, stmt->struc.name->data);
            LLVMTypeRef *types = malloc(sizeof(LLVMTypeRef) * stmt->struc.num_params);
            for (int i = 0; i < stmt->struc.num_params; i++) {
                types[i] = llvm_type(codegen, stmt->struc.args[i].ty);
            }
            LLVMStructSetBody(r, types, stmt->struc.num_params, 0);
            append_llvm_struct_info(codegen->structs, stmt->struc.name, r, stmt->struc.args);
            for (int i = 0; i < stmt->struc.methods->size; i++) {
                LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * stmt->struc.methods->methods[i]->num_params);
                for (int j = 0; j < stmt->struc.methods->methods[i]->num_params; j++) {
                    MethodParam param = stmt->struc.methods->methods[i]->args[j];
                    if (param.is_self) {
                        param_types[j] = LLVMPointerType(r, 0);
                        continue;
                    }
                    param_types[j] = llvm_type(codegen, param.ty);
                }
                LLVMTypeRef func_type = LLVMFunctionType(llvm_type(codegen, stmt->struc.methods->methods[i]->ret), param_types, stmt->struc.methods->methods[i]->num_params, 0);
                String* mangled = mangle_struct_method(stmt->struc.name, stmt->struc.methods->methods[i]->name);
                LLVMValueRef func = LLVMAddFunction(codegen->module, mangled->data, func_type);

                for (int j = 0; j < stmt->struc.methods->methods[i]->num_params; j++) {
                    LLVMValueRef arg = LLVMGetParam(func, j);
                    MethodParam param = stmt->struc.methods->methods[i]->args[j];
                    if (param.is_self) {
                        LLVMSetValueName2(arg, "self", 4);
                        append_llvm_var_info(codegen->vars, new_string("self"), arg, LLVMPointerType(r, 0));
                        continue;
                    }
                    LLVMSetValueName2(arg, param.name->data, strlen(param.name->data));
                    append_llvm_var_info(codegen->vars, param.name, arg, llvm_type(codegen, param.ty));
                }

                append_llvm_var_info(codegen->vars, mangled, func, func_type);
                LLVMBasicBlockRef basic = LLVMAppendBasicBlock(func, "entry");
                codegen->current_function = func;
                LLVMPositionBuilderAtEnd(codegen->builder, basic);
                llvm_codegen_stmt(codegen, stmt->struc.methods->methods[i]->body);
                codegen->vars = filter_llvm_var_info(codegen->vars);
            }
            break;
        }

        case STMT_IF: {
            LLVMValueRef cond = llvm_codegen_expr(codegen, stmt->if_stmt.cond);
            LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(codegen->current_function, "then");
            if (stmt->if_stmt.else_body != NULL) {
                LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(codegen->current_function, "else");
                LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(codegen->current_function, "merge");
                LLVMBuildCondBr(codegen->builder, cond, then_block, else_block);
                LLVMPositionBuilderAtEnd(codegen->builder, then_block);
                llvm_codegen_stmt(codegen, stmt->if_stmt.body);
                LLVMBuildBr(codegen->builder, merge_block);
                LLVMPositionBuilderAtEnd(codegen->builder, else_block);
                llvm_codegen_stmt(codegen, stmt->if_stmt.else_body);
                LLVMBuildBr(codegen->builder, merge_block);
                LLVMPositionBuilderAtEnd(codegen->builder, merge_block);
            } else {
                LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(codegen->current_function, "merge");
                LLVMBuildCondBr(codegen->builder, cond, then_block, merge_block);
                LLVMPositionBuilderAtEnd(codegen->builder, then_block);
                llvm_codegen_stmt(codegen, stmt->if_stmt.body);
                LLVMBuildBr(codegen->builder, merge_block);
                LLVMPositionBuilderAtEnd(codegen->builder, merge_block);
            }
            break;
        }
        case STMT_IMPORT: {
            break;
        }
        case STMT_MODULE: {
            printf("Module %s\n", stmt->module.name->data);
            break;
        }
        default:
            printf("Unknown statement\n");
            break;
    }
}

void llvm_codegen_symbol(LLVMCodeGen *codegen, Symbol *symbol) {
    if (symbol->type->kind == TYPE_FUNC) {
        String *name;
        if (symbol->is_extern) {
            name = symbol->type->func.name;
        } else {
            name = symbol->name;
        }
        LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef) * symbol->type->func.num_params);
        for (int i = 0; i < symbol->type->func.num_params; i++) {
            params[i] = llvm_type(codegen, symbol->type->func.params[i]);
        }

        LLVMTypeRef func_type = LLVMFunctionType(llvm_type(codegen, *symbol->type->func.ret), params,
                                                    symbol->type->func.num_params, 0);
        LLVMValueRef func = LLVMAddFunction(codegen->module, name->data, func_type);
        append_llvm_var_info(codegen->vars, name, func, func_type);

    } else if (symbol->type->kind == TYPE_STRUCT) {
        LLVMTypeRef r = LLVMStructCreateNamed(codegen->context, symbol->name->data);
        LLVMTypeRef *types = malloc(sizeof(LLVMTypeRef) * symbol->type->struc.num_fields);
        for (int i = 0; i < symbol->type->struc.num_fields; i++) {
            types[i] = llvm_type(codegen, symbol->type->struc.fields[i].ty);
        }
        LLVMStructSetBody(r, types, symbol->type->struc.num_fields, 0);
        append_llvm_struct_info(codegen->structs, symbol->name, r, symbol->type->struc.fields);
        for (int i = 0; i < symbol->type->struc.methods->size; i++) {
            LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * symbol->type->struc.methods->methods[i]->num_params);
            for (int j = 0; j < symbol->type->struc.methods->methods[i]->num_params; j++) {
                MethodParam param = symbol->type->struc.methods->methods[i]->args[j];
                if (param.is_self) {
                    param_types[j] = LLVMPointerType(r, 0);
                    continue;
                }
                param_types[j] = llvm_type(codegen, param.ty);
            }
            LLVMTypeRef func_type = LLVMFunctionType(llvm_type(codegen, symbol->type->struc.methods->methods[i]->ret), param_types, symbol->type->struc.methods->methods[i]->num_params, 0);
            String* mangled = mangle_struct_method(symbol->name, symbol->type->struc.methods->methods[i]->name);
            LLVMValueRef func = LLVMAddFunction(codegen->module, mangled->data, func_type);

            for (int k = 0; k < symbol->type->struc.methods->methods[i]->num_params; k++) {
                LLVMValueRef arg = LLVMGetParam(func, k);
                MethodParam param = symbol->type->struc.methods->methods[i]->args[k];
                if (param.is_self) {
                    LLVMSetValueName2(arg, "self", 4);
                    append_llvm_var_info(codegen->vars, new_string("self"), arg, LLVMPointerType(r, 0));
                    continue;
                }
                LLVMSetValueName2(arg, param.name->data, strlen(param.name->data));
                append_llvm_var_info(codegen->vars, param.name, arg, llvm_type(codegen, param.ty));
            }
            LLVMBasicBlockRef basic  = LLVMAppendBasicBlock(func, "entry");
            codegen->current_function = func;
            LLVMPositionBuilderAtEnd(codegen->builder, basic);
            llvm_codegen_stmt(codegen, symbol->type->struc.methods->methods[i]->body);
            append_llvm_var_info(codegen->vars, mangled, func, func_type);
        }
    } else if (symbol->type->kind == TYPE_MODULE) {
//        for (int i = 0; i < symbol->type->module.symbols->size; i++) {
//            Symbol *s = symbol->type->module.symbols->symbols[i];
//            Symbol *ns = new_symbol(mangle_module(symbol->name, s->name), s->type, s->is_extern, s->stmt);
//            codegen->current_module = symbol->name;
//            llvm_codegen_symbol(codegen, ns);
//            codegen->current_module = new_string("");
//        }
    }
}

void llvm_codegen(LLVMCodeGen *codegen) {
    for (int j = 0; j < codegen->symbols->symbols->size; j++) {
        llvm_codegen_symbol(codegen, codegen->symbols->symbols->symbols[j]);
    }
    for (int i = 0; i < codegen->stmts->size; i++) {
        Stmt *stmt = codegen->stmts->stmts[i];
        llvm_codegen_stmt(codegen, stmt);
    }
}

void* llvm_get_function(LLVMCodeGen *codegen, const char *name) {
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllDisassemblers();

    char *error = NULL;
    LLVMExecutionEngineRef engine;
    if (LLVMCreateExecutionEngineForModule(&engine, codegen->module, &error) != 0) {
        fprintf(stderr, "Failed to create execution engine\n");
        return NULL;
    }
    return (void*)LLVMGetFunctionAddress(engine, name);
}
