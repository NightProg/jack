//
// Created by antoine barbier on 20/09/2024.
//

#include "ast.h"
#include "type.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


Symbol *new_symbol(String *name, Type *type, int is_extern, Stmt *stmt) {
    Symbol *symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        return NULL;
    }
    symbol->name = name;
    symbol->type = type;
    symbol->is_extern = is_extern;
    return symbol;
}

void free_symbol(Symbol *symbol) {
    free(symbol);
}

SymbolList *new_symbol_list() {
    SymbolList *list = malloc(sizeof(SymbolList));
    if (list == NULL) {
        return NULL;
    }
    list->symbols = NULL;
    list->size = 0;
    list->capacity = 0;
    return list;
}

int append_symbol(SymbolList *list, Symbol *symbol) {
    if (list->size == list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        Symbol **new_symbols = realloc(list->symbols, sizeof(Symbol*) * new_capacity);
        if (new_symbols == NULL) {
            return 0;
        }
        list->symbols = new_symbols;
        list->capacity = new_capacity;
    }
    list->symbols[list->size++] = symbol;
    return 1;
}

Symbol *find_symbol(SymbolList *list, String *name) {
    for (size_t i = 0; i < list->size; i++) {
        if (compare_strings(list->symbols[i]->name, name) == 0) {
            return list->symbols[i];
        }
    }
    return NULL;
}

void free_symbol_list(SymbolList *list) {
    for (size_t i = 0; i < list->size; i++) {
        free_symbol(list->symbols[i]);
    }
    free(list->symbols);
    free(list);
}

Symbols *new_symbols() {
    Symbols *symbols = malloc(sizeof(Symbols));
    if (symbols == NULL) {
        return NULL;
    }
    symbols->magic = SYMBOLS_MAGIC;
    symbols->version = SYMBOLS_VERSION;
    symbols->symbols = new_symbol_list();
    if (symbols->symbols == NULL) {
        return NULL;
    }
    return symbols;
}

int append_symbols(Symbols *symbols, Symbol *symbol) {
    return append_symbol(symbols->symbols, symbol);
}

Symbol *find_symbols(Symbols *symbols, String *name) {
    return find_symbol(symbols->symbols, name);
}

void free_symbols(Symbols *symbols) {
    free_symbol_list(symbols->symbols);
    free(symbols);
}


Symbols *get_symbols_from_stmts(StmtList *stmts) {
    Symbols *symbols = new_symbols();
    if (symbols == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < stmts->size; i++) {
        Stmt *stmt = stmts->stmts[i];
        if (stmt->type == STMT_FUNCTION) {
            Type* args = malloc(sizeof(Type) * stmt->function.num_params);
            if (args == NULL) {
                free_symbols(symbols);
                return NULL;
            }
            for (int j = 0; j < stmt->function.num_params; j++) {
                args[j] = stmt->function.args[j].ty;
            }

            Type* f = new_func_type(args, stmt->function.num_params, &stmt->function.ret, 0, stmt->function.name, 0);
            if (f == NULL) {
#ifdef NO_GC
                free(args);
                free_symbols(symbols);
#endif
                return NULL;
            }
            Symbol *symbol = new_symbol(stmt->function.name, f, 0, stmt);
            if (symbol == NULL) {
#ifdef NO_GC
                free(args);
                free_symbols(symbols);
#endif
                return NULL;
            }
            if (!append_symbols(symbols, symbol)) {
#ifdef NO_GC
                free(args);
                free_symbols(symbols);
#endif
                return NULL;
            }
        } else if (stmt->type == STMT_EXTERN) {
            Type* args = malloc(sizeof(Type) * stmt->extern_.num_params);
            if (args == NULL) {
                free_symbols(symbols);
                return NULL;
            }
            for (int j = 0; j < stmt->extern_.num_params; j++) {
                args[j] = stmt->extern_.args[j].ty;
            }

            Type* f = new_func_type(args, stmt->extern_.num_params, &stmt->extern_.ret, 0, stmt->extern_.name, 1);
            if (f == NULL) {
                free(args);
                free_symbols(symbols);
                return NULL;
            }
            Symbol *symbol = new_symbol(stmt->extern_.name, f, 1, NULL);
            if (symbol == NULL) {
                free(args);
                free_symbols(symbols);
                return NULL;
            }
            if (!append_symbols(symbols, symbol)) {
                free(args);
                free_symbols(symbols);
                return NULL;
            }
        } else if (stmt->type == STMT_STRUCT) {
            Type* s = new_struct_type(stmt->struc.name, stmt->struc.args, stmt->struc.num_params, stmt->struc.methods,
                                      NULL);
            if (s == NULL) {
                free(s);
                free_symbols(symbols);
                return NULL;
            }
            Symbol *symbol = new_symbol(stmt->struc.name, s, 0, NULL);
            if (symbol == NULL) {
                free_symbols(symbols);
                return NULL;
            }
            if (!append_symbols(symbols, symbol)) {
                free_symbols(symbols);
                return NULL;
            }
        }
    }
    return symbols;
}

Type *new_type(TypeKind kind) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = kind;
    return type;
}

Type *new_func_type(Type *params, int num_params, Type *ret, int is_var_arg, String *name, int is_extern) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = TYPE_FUNC;
    type->parent = new_type(TYPE_INVALID);
    type->func.params = params;
    type->func.num_params = num_params;
    type->func.ret = ret;
    type->func.is_vararg = is_var_arg;
    type->func.name = name;
    type->func.is_extern = is_extern;
    return type;
}

Type *new_struct_type(String *name, Param *fields, int num_fields, MethodList *methods, OpOverloadList *list) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = TYPE_STRUCT;
    type->parent = new_type(TYPE_INVALID);
    type->struc.fields = fields;
    type->struc.num_fields = num_fields;
    type->struc.name = name;
    type->struc.methods = methods;
    type->struc.op_overloads = list;
    return type;
}

Type *new_ptr_type(Type *inner_type) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = TYPE_PTR;
    type->parent = inner_type->parent;
    type->inner_type = inner_type;
    return type;
}

Type *new_array_type(Type *inner_type, int length) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = TYPE_ARRAY;
    type->parent = new_type(TYPE_INVALID);
    type->array.inner_type = inner_type;
    type->array.length = length;
    return type;
}

Type *new_module_type(SymbolList *symbols, String *name) {
    Type *type = malloc(sizeof(Type));
    if (type == NULL) {
        return NULL;
    }
    type->kind = TYPE_MODULE;
    type->parent = new_type(TYPE_INVALID);
    type->module.symbols = symbols;
    type->module.name = name;
    return type;
}

String *display_type(Type* type) {
    String *str = new_string("");
    switch (type->kind) {
        case TYPE_INT:
            add_c_string(str, "int");
            break;
        case TYPE_FLOAT:
            add_c_string(str, "float");
            break;
        case TYPE_STRING:
            add_c_string(str, "string");
            break;
        case TYPE_CHAR:
            add_c_string(str, "char");
            break;
        case TYPE_BOOL:
            add_c_string(str, "bool");
            break;
        case TYPE_FUNC:
            add_c_string(str, "func(");
            for (int i = 0; i < type->func.num_params; i++) {
                add_string(str, display_type(&type->func.params[i]));
                if (i + 1 < type->func.num_params) {
                    add_c_string(str, ", ");
                }
            }
            add_c_string(str, ")");
            add_c_string(str, " : ");
            add_string(str, display_type(type->func.ret));
            break;
        case TYPE_STRUCT:
            add_string(str, type->struc.name);
            break;
        case TYPE_ARRAY:
            add_c_string(str, "[");
            add_string(str, display_type(type->array.inner_type));
            add_c_string(str, "]");
            break;
        case TYPE_PTR:
            add_string(str, display_type(type->inner_type));
            add_c_string(str, "*");
            break;
        case TYPE_NULL:
            add_c_string(str, "null");
            break;
        case TYPE_VOID:
            add_c_string(str, "void");
            break;
        case TYPE_MODULE:
            add_c_string(str, "module ");
            add_string(str, type->module.name);
            break;
        case TYPE_GENERIC:
            add_c_string(str, "generic ");
            add_string(str, type->generic.name);
            break;
        case TYPE_INVALID:
            add_c_string(str, "invalid");
            break;
    }
    return str;
}

void free_type(Type *type) {
    if (type->kind == TYPE_FUNC) {
        free(type->func.params);
    }
    free(type);
}


GenericList *new_generic_list() {
    GenericList *list = malloc(sizeof(GenericList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->types = malloc(sizeof(Type*) * list->capacity);
    if (list->types == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}

void append_generic(GenericList *list, Type *type) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Type** new_types = malloc(sizeof(Type*) * list->capacity);
        if (new_types == NULL) {
            return;
        }
        memcpy(new_types, list->types, sizeof(Type*) * list->size);
        free(list->types);
        list->types = new_types;
    }
    list->types[list->size++] = type;
}

GenericList *generic_list_from_array(int length, Type **array) {
    GenericList *list = new_generic_list();
    for (int i = 0; i < length; i++) {
        append_generic(list, array[i]);
    }
    return list;
}

void free_generic_list(GenericList *list) {
    free(list->types);
    free(list);
}

Method *new_method(String *name, MethodParam *args, int num_params, Type ret, Stmt *body) {
    Method *method = malloc(sizeof(Method));
    if (method == NULL) {
        return NULL;
    }
    method->name = name;
    method->args = args;
    method->num_params = num_params;
    method->ret = ret;
    method->body = body;
    return method;
}

MethodList *new_method_list() {
    MethodList *list = malloc(sizeof(MethodList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->methods = malloc(sizeof(Method*) * list->capacity);
    if (list->methods == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}

void append_method(MethodList *list, Method *method) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Method** new_methods = malloc(sizeof(Method*) * list->capacity);
        if (new_methods == NULL) {
            return;
        }
        memcpy(new_methods, list->methods, sizeof(Method*) * list->size);
        free(list->methods);
        list->methods = new_methods;
    }
    list->methods[list->size++] = method;
}

Method *find_method(MethodList *list, String *name) {
    for (int i = 0; i < list->size; i++) {
        if (compare_strings(list->methods[i]->name, name) == 0) {
            return list->methods[i];
        }
    }
    return NULL;
}

MethodList *method_list_from_array(int length, Method **array) {
    MethodList *list = new_method_list();
    for (int i = 0; i < length; i++) {
        append_method(list, array[i]);
    }
    return list;
}

void free_method_list(MethodList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->methods[i]);
    }
    free(list->methods);
    free(list);
}

Expr *new_int_expr(int val, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_INT;
    expr->span = span;
    expr->int_val = val;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_float_expr(float val, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_FLOAT;
    expr->span = span;
    expr->float_val = val;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_string_expr(String *val, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_STRING;
    expr->span = span;
    expr->string_val = val;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_char_expr(char val, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_CHAR;
    expr->span = span;
    expr->char_val = val;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_ident_expr(String *val, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_IDENT;
    expr->span = span;
    expr->ident_val = val;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_binop_expr(int op, Expr *lhs, Expr *rhs, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_BINOP;
    expr->binop.op = op;
    expr->binop.lhs = lhs;
    expr->binop.rhs = rhs;
    expr->span = span;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_unop_expr(int op, Expr *expr, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_UNOP;
    new_expr->unop.op = op;
    new_expr->unop.expr = expr;
    new_expr->span = span;
    expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_call_expr(Expr *name, ExprList *args, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_CALL;
    expr->call.name = name;
    expr->call.args = args;
    expr->span = span;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_init_expr(Expr *name, ExprList *fields, Span span) {
    Expr *expr = malloc(sizeof(Expr));
    if (expr == NULL) {
        return NULL;
    }
    expr->type = EXPR_INIT;
    expr->init.name = name;
    expr->init.fields = fields;
    expr->span = span;
    expr->ty = new_type(TYPE_INVALID);
    return expr;
}

Expr *new_get_expr(Expr *expr, String *field, int is_ptr, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_GET;
    new_expr->get.expr = expr;
    new_expr->get.field = field;
    new_expr->get.is_ptr = is_ptr;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_assign_expr(Expr *var, Expr *val, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_ASSIGN;
    new_expr->assign.var = var;
    new_expr->assign.val = val;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}


Expr *new_ref_expr(Expr *expr, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_REF;
    new_expr->ref = expr;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_deref_expr(Expr *expr, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_DEREF;
    new_expr->deref = expr;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_array_expr(ExprList *exprs, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_ARRAY;
    new_expr->array.exprs = exprs;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_null_expr(Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_NULL;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_as_expr(Expr *expr, Type ty, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_AS;
    new_expr->as.expr = expr;
    new_expr->as.ty = ty;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_index_expr(Expr *list, Expr *index, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_INDEX;
    new_expr->index.list = list;
    new_expr->index.index = index;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

Expr *new_ns_expr(StringList *path, Span span) {
    Expr *new_expr = malloc(sizeof(Expr));
    if (new_expr == NULL) {
        return NULL;
    }
    new_expr->type = EXPR_NS;
    new_expr->ns.path = path;
    new_expr->span = span;
    new_expr->ty = new_type(TYPE_INVALID);
    return new_expr;
}

int is_arithmetic_op(int op) {
    return op == BINOP_ADD || op == BINOP_SUB || op == BINOP_MUL || op == BINOP_DIV;
}

int is_cmp_op(int op) {
    return op == BINOP_EQ || op == BINOP_NEQ || op == BINOP_LT || op == BINOP_GT || op == BINOP_LTE || op == BINOP_GTE;
}

ExprList *new_expr_list() {
    ExprList *list = malloc(sizeof(ExprList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->exprs = malloc(sizeof(Expr*) * list->capacity);
    if (list->exprs == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}



int remove_expr(ExprList *list, int index) {
    if (index < 0 || index >= list->size) {
        return -1;
    }
    for (int i = index; i < list->size - 1; i++) {
        list->exprs[i] = list->exprs[i + 1];
    }
    list->size--;
    return 0;
}

void append_expr(ExprList *list, Expr *expr) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Expr** new_exprs = malloc(sizeof(Expr*) * list->capacity);
        if (new_exprs == NULL) {
            return;
        }
        memcpy(new_exprs, list->exprs, sizeof(Expr*) * list->size);
        free(list->exprs);
        list->exprs = new_exprs;
    }
    list->exprs[list->size++] = expr;
}

void append_expr_at_first(ExprList *list, Expr *expr) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Expr** new_exprs = malloc(sizeof(Expr*) * list->capacity);
        if (new_exprs == NULL) {
            return;
        }
        memcpy(new_exprs + 1, list->exprs, sizeof(Expr*) * list->size);
        free(list->exprs);
        list->exprs = new_exprs;
    } else {
        for (int i = list->size; i > 0; i--) {
            list->exprs[i] = list->exprs[i - 1];
        }
    }
    list->exprs[0] = expr;
    list->size++;
}

ExprList *expr_list_from_array(int length, Expr **array) {
    ExprList *list = new_expr_list();
    for (int i = 0; i < length; i++) {
        append_expr(list, array[i]);
    }
    return list;
}

void free_expr_list(ExprList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->exprs[i]);
    }
    free(list->exprs);
    free(list);
}

OpOverload *new_unop_overload(Method *method, int unary_op) {
    OpOverload *overload = malloc(sizeof(OpOverload));
    if (overload == NULL) {
        return NULL;
    }
    overload->kind = OP_OVERLOAD_UNOP;
    overload->method = method;
    overload->unop = unary_op;
    return overload;
}

OpOverload *new_binop_overload(Method *method, int binary_op) {
    OpOverload *overload = malloc(sizeof(OpOverload));
    if (overload == NULL) {
        return NULL;
    }
    overload->kind = OP_OVERLOAD_BINOP;
    overload->method = method;
    overload->binop = binary_op;
    return overload;
}

OpOverloadList *new_op_overload_list() {
    OpOverloadList *list = malloc(sizeof(OpOverloadList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->overloads = malloc(sizeof(OpOverload*) * list->capacity);
    if (list->overloads == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}

void append_op_overload(OpOverloadList *list, OpOverload *overload) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        OpOverload** new_overloads = malloc(sizeof(OpOverload*) * list->capacity);
        if (new_overloads == NULL) {
            return;
        }
        memcpy(new_overloads, list->overloads, sizeof(OpOverload*) * list->size);
        free(list->overloads);
        list->overloads = new_overloads;
    }
    list->overloads[list->size++] = overload;
}

OpOverload *find_op_overload(OpOverloadList *list, OpOverloadKind kind, int op) {
    for (int i = 0; i < list->size; i++) {
        if (list->overloads[i]->kind == kind) {
            if (kind == OP_OVERLOAD_UNOP) {
                if (list->overloads[i]->unop == op) {
                    return list->overloads[i];
                }
            } else {
                if (list->overloads[i]->binop == op) {
                    return list->overloads[i];
                }
            }
        }
    }
    return NULL;
}

void free_op_overload_list(OpOverloadList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->overloads[i]);
    }
    free(list->overloads);
    free(list);
}

Extension *new_extension(Type ty, MethodList *list) {
    Extension *extension = malloc(sizeof(Extension));
    if (extension == NULL) {
        return NULL;
    }
    extension->ty = ty;
    extension->methods = list;
    return extension;
}

ExtensionList *new_extension_list() {
    ExtensionList *list = malloc(sizeof(ExtensionList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->extensions = malloc(sizeof(Extension*) * list->capacity);
    if (list->extensions == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}

void append_extension(ExtensionList *list, Extension *extension) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Extension** new_extensions = malloc(sizeof(Extension*) * list->capacity);
        if (new_extensions == NULL) {
            return;
        }
        memcpy(new_extensions, list->extensions, sizeof(Extension*) * list->size);
        free(list->extensions);
        list->extensions = new_extensions;
    }
    list->extensions[list->size++] = extension;
}

ExtensionList *extension_list_from_array(int length, Extension **array) {
    ExtensionList *list = new_extension_list();
    for (int i = 0; i < length; i++) {
        append_extension(list, array[i]);
    }
    return list;
}

Extension *find_extension(ExtensionList *list, Type *ty) {
    for (int i = 0; i < list->size; i++) {
        if (check_same_type(&list->extensions[i]->ty, ty)) {
            return list->extensions[i];
        }
    }
    return NULL;
}

void append_or_set_extension(ExtensionList *list, Extension *extension) {
    Extension *ext = find_extension(list, &extension->ty);
    if (ext == NULL) {
        append_extension(list, extension);
    } else {
        ext->methods = extension->methods;
    }
}

void free_extension_list(ExtensionList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->extensions[i]);
    }
    free(list->extensions);
    free(list);
}


StmtList *new_stmt_list() {
    StmtList *list = malloc(sizeof(StmtList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->stmts = malloc(sizeof(Stmt*) * list->capacity);
    if (list->stmts == NULL) {
        free(list);
        printf("Failed to allocate memory\n");
        return NULL;
    }
    return list;
}

int remove_stmt(StmtList *list, int index) {
    if (index < 0 || index >= list->size) {
        return -1;
    }
    for (int i = index; i < list->size - 1; i++) {
        list->stmts[i] = list->stmts[i + 1];
    }
    list->size--;
    return 0;
}

void append_stmt(StmtList *list, Stmt *stmt) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Stmt** new_stmts = malloc(sizeof(Stmt*) * list->capacity);
        if (new_stmts == NULL) {
            return;
        }
        memcpy(new_stmts, list->stmts, sizeof(Stmt*) * list->size);
        free(list->stmts);
        list->stmts = new_stmts;
    }
    list->stmts[list->size++] = stmt;
}

int append_stmt_at_first(StmtList *list, Stmt *expr) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Stmt ** new_stmts = malloc(sizeof(Stmt *) * list->capacity);
        if (new_stmts == NULL) {
            return 0;
        }
        memcpy(new_stmts + 1, list->stmts, sizeof(Stmt *) * list->size);
        free(list->stmts);
        list->stmts = new_stmts;
    } else {
        for (int i = list->size; i > 0; i--) {
            list->stmts[i] = list->stmts[i - 1];
        }
    }
    list->stmts[0] = expr;
    list->size++;
    return 1;
}

int replace_stmts(StmtList *list, Stmt *to_be_replaced, Stmt *stmt) {
    for (int i = 0; i < list->size; i++) {
        if (list->stmts[i] == to_be_replaced) {
            list->stmts[i] = stmt;
            return 1;
        }
    }
    return 0;
}

int replace_stmt(StmtList *list, int index, Stmt *stmt) {
    if (index < 0 || index >= list->size) {
        return 0;
    }
    list->stmts[index] = stmt;
    return 1;
}

int find_stmt(StmtList *list, Stmt *stmt) {
    for (int i = 0; i < list->size; i++) {
        if (list->stmts[i] == stmt) {
            return i;
        }
    }
    return -1;
}


StmtList *stmt_list_from_array(int length, Stmt **array) {
    StmtList *list = new_stmt_list();
    for (int i = 0; i < length; i++) {
        append_stmt(list, array[i]);
    }
    return list;
}

void free_stmt_list(StmtList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->stmts[i]);
    }
    free(list->stmts);
    free(list);
}


Type get_struct_type_from_stmt(Stmt *stmt) {
    if (stmt->type == STMT_STRUCT) {
        return *new_struct_type(stmt->struc.name, stmt->struc.args, stmt->struc.num_params, stmt->struc.methods, stmt->struc.overloads);
    }
    return *new_type(TYPE_INVALID);
}

Stmt *new_expr_stmt(Expr *expr, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_EXPR;
    stmt->span = span;
    stmt->expr = expr;
    return stmt;
}

Stmt *new_struct_stmt(String *name, Param *args, int num_params, MethodList *methods, OpOverloadList *op_overloads,
                      Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_STRUCT;
    stmt->span = span;
    stmt->struc.name = name;
    stmt->struc.args = args;
    stmt->struc.num_params = num_params;
    stmt->struc.methods = methods;
    stmt->struc.overloads = op_overloads;
    return stmt;
}

Stmt *new_if_stmt(Expr *cond, Stmt *body, Span span, Stmt *else_body) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_IF;
    stmt->span = span;
    stmt->if_stmt.cond = cond;
    stmt->if_stmt.body = body;
    stmt->if_stmt.else_body = else_body;
    return stmt;
}

Stmt *new_while_stmt(Expr *cond, Stmt *body, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_WHILE;
    stmt->span = span;
    stmt->while_stmt.cond = cond;
    stmt->while_stmt.body = body;
    return stmt;
}

Stmt *new_for_stmt(Expr *init, Expr *cond, Expr *update, Stmt *body, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_FOR;
    stmt->span = span;
    stmt->for_stmt.init = init;
    stmt->for_stmt.cond = cond;
    stmt->for_stmt.update = update;
    stmt->for_stmt.body = body;
    return stmt;
}

Stmt *new_block_stmt(StmtList *stmts, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_BLOCK;
    stmt->span = span;
    stmt->block.stmts = stmts;
    return stmt;
}

Stmt *new_let_stmt(String *name, Type *type, Span span, Expr *expr) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_LET;
    stmt->span = span;
    stmt->let.name = name;
    stmt->let.type = type;
    stmt->let.expr = expr;
    return stmt;
}

Stmt *new_return_stmt(Expr *expr, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_RETURN;
    stmt->span = span;
    stmt->ret_expr = expr;
    return stmt;
}

Stmt *
new_function_stmt(String *name, GenericList *list, Param *param, int num_params, Stmt *body, Span span, Type ret) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_FUNCTION;
    stmt->span = span;
    stmt->function.generics = list;
    stmt->function.name = name;
    stmt->function.args = param;
    stmt->function.num_params = num_params;
    stmt->function.body = body;
    stmt->function.ret = ret;
    return stmt;
}

Stmt *new_extern_stmt(String *name, Param *param, int num_params, Type ret, int is_vararg, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_EXTERN;
    stmt->span = span;
    stmt->extern_.name = name;
    stmt->extern_.args = param;
    stmt->extern_.ret = ret;
    stmt->extern_.num_params = num_params;
    stmt->extern_.is_vararg = is_vararg;

    return stmt;
}

Stmt *new_module_stmt(String *name, StmtList *stmts, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_MODULE;
    stmt->span = span;
    stmt->module.name = name;
    stmt->module.stmts = stmts;
    return stmt;
}

Stmt *new_import_stmt(String *name, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_IMPORT;
    stmt->span = span;
    stmt->import.name = name;
    return stmt;
}

Stmt *new_extension_stmt(Extension *extension, Span span) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (stmt == NULL) {
        return NULL;
    }
    stmt->type = STMT_EXTENSION;
    stmt->span = span;
    stmt->extension = extension;
    return stmt;
}

