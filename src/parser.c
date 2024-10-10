#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void init_parser(Parser *parser, String *source, TokenList *tokens) {
    errorList = malloc(sizeof(ErrorList));
    if (errorList == NULL) {
        return;
    }
    init_error_list(errorList);
    parser->tokens = tokens;
    parser->source = source;
    parser->current = 0;
}

void free_parser(Parser *parser) {
    free_token_list(parser->tokens);
    free(parser);
}

int is_eof(Parser *parser) {
    return parser->current >= parser->tokens->size;
}

int check(Parser *parser, TokenType type) {
    if (is_eof(parser)) {
        return 0;
    }
    return parser->tokens->tokens[parser->current]->type == type;
}

Expr *parse_expr(Parser *parser) {
    return parse_term(parser);
}

Expr *parse_term(Parser *parser) {
    if (is_eof(parser)) {
        add_error(errorList, "Unexpected end of file", parser->tokens->tokens[parser->current - 1]->span, parser->source);
        return NULL;
    }
    Expr *left = parse_factor(parser);
    if (left == NULL) {
        return NULL;
    }
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *right = parse_term(parser);
        if (right == NULL) {
            return NULL;
        }
        if (op->type == TOKEN_PLUS) {
            left = new_binop_expr(BINOP_ADD, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_MINUS) {
            left = new_binop_expr(BINOP_SUB, left, right, extend_span(parser->source, left->span, right->span));
        }
    }
    return left;
}

Expr *parse_factor(Parser *parser) {
    Expr *left = parse_cmp(parser);
    if (left == NULL) {
        return NULL;
    }
    while (check(parser, TOKEN_MUL) || check(parser, TOKEN_DIV)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *right = parse_factor(parser);
        if (right == NULL) {
            return NULL;
        }
        if (op->type == TOKEN_MUL) {
            left = new_binop_expr(BINOP_MUL, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_DIV) {
            left = new_binop_expr(BINOP_DIV, left, right, extend_span(parser->source, left->span, right->span));
        }
    }
    return left;
}

Expr *parse_cmp(Parser *parser) {
    Expr *left = parse_unary(parser);
    if (left == NULL) {
        return NULL;
    }
    while (check(parser, TOKEN_EQ) || check(parser, TOKEN_NEQ) || check(parser, TOKEN_LT) || check(parser, TOKEN_GT) || check(parser, TOKEN_LTE) || check(parser, TOKEN_GTE)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *right = parse_cmp(parser);
        if (right == NULL) {
            return NULL;
        }
        if (op->type == TOKEN_EQ) {
            left = new_binop_expr(BINOP_EQ, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_NEQ) {
            left = new_binop_expr(BINOP_NEQ, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_LT) {
            left = new_binop_expr(BINOP_LT, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_GT) {
            left = new_binop_expr(BINOP_GT, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_LTE) {
            left = new_binop_expr(BINOP_LTE, left, right, extend_span(parser->source, left->span, right->span));
        }
        if (op->type == TOKEN_GTE) {
            left = new_binop_expr(BINOP_GTE, left, right, extend_span(parser->source, left->span, right->span));
        }
    }
    return left;
}

Expr *parse_unary(Parser *parser) {
    if (check(parser, TOKEN_MINUS) || check(parser, TOKEN_NOT)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *expr = parse_as(parser);
        if (expr == NULL) {
            return NULL;
        }
        if (op->type == TOKEN_MINUS) {
            return new_unop_expr(UNOP_NEG, expr, extend_span(parser->source, op->span, expr->span));
        }
        if (op->type == TOKEN_NOT) {
            return new_unop_expr(UNOP_NOT, expr, extend_span(parser->source, op->span, expr->span));
        }
    }
    return parse_as(parser);
}

Expr *parse_as(Parser *parser) {
    Expr *expr = parse_assign(parser);
    if (expr == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_AS)) {
        parser->current++;
        Type ty = parse_type(parser);
        if (ty.kind == TYPE_INVALID) {
            return NULL;
        }
        return new_as_expr(expr, ty, extend_span(parser->source, expr->span, parser->tokens->tokens[parser->current - 1]->span));
    }
    return expr;
}

Expr *parse_assign(Parser *parser) {
    Expr *expr = parse_index(parser);
    if (expr == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_ASSIGN)) {
        parser->current++;
        Expr *val = parse_expr(parser);
        if (val == NULL) {
            return NULL;
        }
        return new_assign_expr(expr, val, extend_span(parser->source, expr->span, val->span));
    }
    return expr;
}

Expr *parse_index(Parser *parser) {
    Expr *expr = parse_call(parser);
    if (expr == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_LBRACKET)) {
        parser->current++;
        Expr *index = parse_expr(parser);
        if (index == NULL) {
            return NULL;
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACKET) {
            add_error(errorList, "Expected ']'", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        parser->current++;
        return new_index_expr(expr, index, extend_span(parser->source, expr->span, index->span));
    }
    return expr;
}

Expr *parse_call(Parser *parser) {
    Expr *expr = parse_ref(parser);
    if (expr == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_LPAREN)) {
        parser->current++;
        ExprList *args = new_expr_list();
        while (!check(parser, TOKEN_RPAREN)) {
            Expr *arg = parse_expr(parser);
            if (arg == NULL) {
                return NULL;
            }
            append_expr(args, arg);
            if (parser->tokens->tokens[parser->current]->type == TOKEN_COMMA) {
                parser->current++;
            }
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
            add_error(errorList, "Expected ')'", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        parser->current++;
        return new_call_expr(expr, args, extend_span(parser->source, expr->span, parser->tokens->tokens[parser->current - 1]->span));
    } else {
        return expr;
    }
}


Expr *parse_ref(Parser *parser) {
    if (check(parser, TOKEN_AMP)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *expr = parse_deref(parser);
        if (expr == NULL) {
            return NULL;
        }
        return new_ref_expr(expr, extend_span(parser->source, op->span, expr->span));
    }
    return parse_deref(parser);
}

Expr *parse_deref(Parser *parser) {
    if (check(parser, TOKEN_MUL)) {
        Token *op = parser->tokens->tokens[parser->current];
        parser->current++;
        Expr *expr = parse_get(parser);
        if (expr == NULL) {
            return NULL;
        }
        return new_deref_expr(expr, extend_span(parser->source, op->span, expr->span));
    }
    return parse_get(parser);
}

Expr *parse_get(Parser *parser) {
    Expr *expr = parse_init(parser);
    if (expr == NULL) {
        return NULL;
    }
    while (check(parser, TOKEN_DOT) || check(parser, TOKEN_ARROW)) {
        int is_ptr = 0;
        if (check(parser, TOKEN_ARROW)) {
            is_ptr = 1;
        }
        parser->current++;
        if (!check(parser, TOKEN_IDENTIFIER)) {
            add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }

        Token *field = parser->tokens->tokens[parser->current];
        parser->current++;
        expr = new_get_expr(expr, field->span.source, is_ptr, extend_span(parser->source, expr->span, field->span));
    }
    return expr;
}

Expr *parse_init(Parser *parser) {
    Expr *name = parse_primary(parser);
    if (name == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_LBRACE)) {
        parser->current++;
        ExprList *fields = new_expr_list();
        while (!check(parser, TOKEN_RBRACE)) {
            if (fields->size > 0) {
                if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                    add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                    return NULL;
                }
                parser->current++;
            }
            Expr *field = parse_expr(parser);
            if (field == NULL) {
                return NULL;
            }
            append_expr(fields, field);
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACE) {
            add_error(errorList, "Expected '}'", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        parser->current++;
        return new_init_expr(name, fields, extend_span(parser->source, name->span, parser->tokens->tokens[parser->current - 1]->span));
    }
    return name;
}



Expr *parse_primary(Parser *parser) {
    Token *token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (token->type == TOKEN_NUMBER) {
        return new_int_expr(atoi(token->span.source->data), token->span);
    }
    if (token->type == TOKEN_IDENTIFIER) {
        if (check(parser, TOKEN_DOUBLE_COLON)) {
            StringList *path = new_string_list();
            append_string(path, token->span.source);
            while(check(parser, TOKEN_DOUBLE_COLON)) {
                parser->current++;
                if (!check(parser, TOKEN_IDENTIFIER)) {
                    add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
                    return NULL;
                }
                append_string(path, parser->tokens->tokens[parser->current]->span.source);
                parser->current++;
            }
            return new_ns_expr(path, extend_span(parser->source, token->span, parser->tokens->tokens[parser->current - 1]->span));

        }
        return new_ident_expr(token->span.source, token->span);
    }
    if (token->type == TOKEN_LPAREN) {
        Expr *expr = parse_expr(parser);
        if (expr == NULL) {
            add_error(errorList, "Expected expression", token->span, parser->source);
            return NULL;
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
            add_error(errorList, "Expected ')'", token->span, parser->source);
            return NULL;
        }
        parser->current++;
        return expr;
    }
    if (token->type == TOKEN_STRING) {
        return new_string_expr(token->span.source, token->span);
    }
    if (token->type == TOKEN_LPAREN) {
        Expr *expr = parse_expr(parser);
        if (expr == NULL) {
            return NULL;
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
            add_error(errorList, "Expected ')'", token->span, parser->source);
            return NULL;
        }
        parser->current++;
        return expr;
    }
    if (token->type == TOKEN_NULL) {
        return new_null_expr(token->span);
    }
    if (token->type == TOKEN_CHAR) {
        return new_char_expr(token->span.source->data[1], token->span);
    }
    if (token->type == TOKEN_SELF) {
        return new_ident_expr(token->span.source, token->span);
    }
    if (token->type == TOKEN_LBRACKET) {
        ExprList *exprs = new_expr_list();
        while (!check(parser, TOKEN_RBRACKET)) {
            if (exprs->size > 0) {
                if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                    add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                    return NULL;
                }
                parser->current++;
            }
            Expr *expr = parse_expr(parser);
            if (expr == NULL) {
                return NULL;
            }
            append_expr(exprs, expr);
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACKET) {
            add_error(errorList, "Expected ']'", token->span, parser->source);
            return NULL;
        }
        parser->current++;
        return new_array_expr(exprs, extend_span(parser->source, token->span, parser->tokens->tokens[parser->current - 1]->span));
    }
    add_error(errorList, "Expected expression", token->span, parser->source);
    return NULL;
}

Param parse_param(Parser *parser) {
    if (parser->tokens->tokens[parser->current]->type != TOKEN_IDENTIFIER) {
        add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
        return (Param) {NULL, *new_type(TYPE_INVALID)};
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (parser->tokens->tokens[parser->current]->type != TOKEN_COL) {
        add_error(errorList, "Expected ':'", ident->span, parser->source);
        return (Param) {NULL, *new_type(TYPE_INVALID)};
    }
    parser->current++;
    Type ty = parse_type(parser);
    if (ty.kind == TYPE_INVALID) {
        return (Param) {NULL, *new_type(TYPE_INVALID)};
    }
    return (Param) {ident->span.source, ty};
}

Method *parse_method(Parser *parser) {
    if (!check(parser, TOKEN_FUNC)) {
        add_error(errorList, "Expected 'func'", parser->tokens->tokens[parser->current]->span, parser->source);
        return NULL;
    }
    parser->current++;
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
        return NULL;
    }
    Token *method_name = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", method_name->span, parser->source);
        return NULL;
    }
    parser->current++;
    MethodParam *args = malloc(sizeof(MethodParam));
    int num_params = 0;
    while (!check(parser, TOKEN_RPAREN)) {
        if (num_params > 0) {
            if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                return NULL;
            }
            parser->current++;
        }

        if (check(parser, TOKEN_SELF)) {
            if (num_params > 0) {
                add_error(errorList, "self should always be the first argument", parser->tokens->tokens[parser->current]->span, parser->source);
                return NULL;
            }
            parser->current++;
            args = realloc(args, sizeof(MethodParam) * (num_params + 1));
            int is_ptr = 0;
            if (check(parser, TOKEN_MUL)) {
                parser->current++;
                is_ptr = 1;
            }
            args[num_params] = (MethodParam) {1, is_ptr, NULL, *new_type(TYPE_INVALID)};
            num_params++;
            continue;
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_IDENTIFIER) {
            add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        Token *param_name = parser->tokens->tokens[parser->current];
        parser->current++;
        if (parser->tokens->tokens[parser->current]->type != TOKEN_COL) {
            add_error(errorList, "Expected ':'", param_name->span, parser->source);
            return NULL;
        }
        parser->current++;
        Type ty = parse_type(parser);
        if (ty.kind == TYPE_INVALID) {
            return NULL;
        }
        args = realloc(args, sizeof(MethodParam) * (num_params + 1));
        args[num_params] = (MethodParam) {0, 0, param_name->span.source, ty};
        num_params++;
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
        add_error(errorList, "Expected ')'", method_name->span, parser->source);
        return NULL;
    }
    parser->current++;
    if (parser->tokens->tokens[parser->current]->type != TOKEN_COL) {
        add_error(errorList, "Expected ':'", method_name->span, parser->source);
        return NULL;
    }
    parser->current++;
    Type ret = parse_type(parser);
    if (ret.kind == TYPE_INVALID) {
        return NULL;
    }
    Stmt *body = parse_stmt(parser);
    if (body == NULL) {
        return NULL;
    }
    return new_method(method_name->span.source, args, num_params, ret, body);
}

Stmt *parse_stmt(Parser *parser) {
    if (check(parser, TOKEN_IF)) {
        return parse_if(parser);
    }
    if (check(parser, TOKEN_WHILE)) {
        return parse_while(parser);
    }
    if (check(parser, TOKEN_FOR)) {
        return parse_for(parser);
    }
    if (check(parser, TOKEN_LBRACE)) {
        return parse_block(parser);
    }
    if (check(parser, TOKEN_LET)) {
        return parse_let(parser);
    }
    if (check(parser, TOKEN_FUNC)) {
        return parse_func(parser);
    }
    if (check(parser, TOKEN_RETURN)) {
        return parse_return(parser);
    }
    if (check(parser, TOKEN_STRUCT)) {
        return parse_struct(parser);
    }
    if (check(parser, TOKEN_EXTERN)) {
        return parse_extern(parser);
    }
    if (check(parser, TOKEN_IMPORT)) {
        return parse_import(parser);
    }
    if (check(parser, TOKEN_MODULE)) {
        return parse_module(parser);
    }
    if (check(parser, TOKEN_EXTENSION)) {
        return parse_extension(parser);
    }

    Expr *expr = parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }
    Stmt *stmt = new_expr_stmt(expr, expr->span);
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", expr->span, parser->source);
        return NULL;
    }
    parser->current++;
    return stmt;
}

OpOverload *parse_op_overload(Parser *parser) {
    if (!check(parser, TOKEN_OP)) {
        add_error(errorList, "Expected 'op'", parser->tokens->tokens[parser->current]->span, parser->source);
        return NULL;
    }
    parser->current++;
    Token *top = parser->tokens->tokens[parser->current];
    parser->current++;
    int op;
    switch (top->type) {
        case TOKEN_PLUS: {
            op = BINOP_ADD;
            break;
        }
        case TOKEN_MINUS: {
            op = BINOP_SUB;
            break;
        }
        case TOKEN_MUL: {
            op = BINOP_MUL;
            break;
        }
        case TOKEN_DIV: {
            op = BINOP_DIV;
            break;
        }
        case TOKEN_EQ: {
            op = BINOP_EQ;
            break;
        }
        case TOKEN_NEQ: {
            op = BINOP_NEQ;
            break;
        }
        case TOKEN_LT: {
            op = BINOP_LT;
            break;
        }
        case TOKEN_GT: {
            op = BINOP_GT;
            break;
        }
        case TOKEN_LTE: {
            op = BINOP_LTE;
            break;
        }
        case TOKEN_GTE: {
            op = BINOP_GTE;
            break;
        }
        default: {
            add_error(errorList, "Invalid operator", top->span, parser->source);
            return NULL;
        }
    }

    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", top->span, parser->source);
        return NULL;
    }
    if (is_arithmetic_op(op)) {
        // self, and other
        parser->current++;
        if (!check(parser, TOKEN_SELF)) {
            add_error(errorList, "Expected 'self'", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }

        parser->current++;
        if (!check(parser, TOKEN_COMMA)) {
            add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        parser->current++;
        if (!check(parser, TOKEN_IDENTIFIER)) {
            add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        Token *other = parser->tokens->tokens[parser->current];
        parser->current++;
        if (!check(parser, TOKEN_COL)) {
            add_error(errorList, "Expected ':'", other->span, parser->source);
            return NULL;
        }
        parser->current++;
        Type ty = parse_type(parser);
        if (!check(parser, TOKEN_RPAREN)) {
            add_error(errorList, "Expected ')'", other->span, parser->source);
            return NULL;
        }
        parser->current++;
        if (!check(parser, TOKEN_COL)) {
            add_error(errorList, "Expected ':'", other->span, parser->source);
            return NULL;
        }
        parser->current++;
        Type ret = parse_type(parser);
        Stmt *body = parse_stmt(parser);
        MethodParam *args = malloc(sizeof(MethodParam) * 2);
        args[0] = (MethodParam) {1, 0, NULL, *new_type(TYPE_INVALID)};
        args[1] = (MethodParam) {0, 0, other->span.source, ty};
        Method *method = new_method(top->span.source, args, 2, ret, body);
        return new_binop_overload(method, op);
    } else {
#include <assert.h>
        assert(1);
    }
}

Stmt *parse_struct(Parser *parser) {
    Token *struct_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", struct_token->span, parser->source);
        return NULL;
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LBRACE)) {
        add_error(errorList, "Expected '{'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Param *fields = malloc(sizeof(Param));
    int num_fields = 0;
    while (!check(parser, TOKEN_SEMICOLON)) {
        if (num_fields > 0) {
            if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                return NULL;
            }
            parser->current++;
        }
        Param field = parse_param(parser);
        if (field.name == NULL) {
            return NULL;
        }
        fields = realloc(fields, sizeof(Param) * (num_fields + 1));
        fields[num_fields] = field;
        num_fields++;
    }
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    MethodList *methods = new_method_list();
    OpOverloadList *op_overloads = new_op_overload_list();
    while (!check(parser, TOKEN_RBRACE)) {
        if (check(parser, TOKEN_OP)) {
            OpOverload *op_overload = parse_op_overload(parser);
            if (op_overload == NULL) {
                return NULL;
            }
            append_op_overload(op_overloads, op_overload);
            continue;
        }
        Method *method = parse_method(parser);
        if (method == NULL) {
            return NULL;
        }
        append_method(methods, method);
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACE) {
        add_error(errorList, "Expected '}'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_struct_stmt(ident->span.source, fields, num_fields, methods, op_overloads,
                           extend_span(parser->source, struct_token->span,
                                       parser->tokens->tokens[
                                               parser->current -
                                               1]->span));
}

Stmt *parse_return(Parser *parser) {
    Token *return_token = parser->tokens->tokens[parser->current];
    parser->current++;
    Expr *expr = parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }
    return new_return_stmt(expr, extend_span(parser->source, return_token->span, expr->span));
}

Type parse_type(Parser *parser) {
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", parser->tokens->tokens[parser->current]->span, parser->source);
        return *new_type(TYPE_INVALID);
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    int is_ptr = 0;
    if (check(parser, TOKEN_MUL)) {
        parser->current++;
        is_ptr = 1;
    }
    Type* res = new_type(TYPE_INVALID);
    if (compare_string(ident->span.source, "int") == 0) {
        *res = *new_type(TYPE_INT);
    } else if (compare_string(ident->span.source, "string") == 0) {
        *res = *new_type(TYPE_STRING);
    } else if (compare_string(ident->span.source, "void") == 0) {
        *res = *new_type(TYPE_VOID);
    } else if (compare_string(ident->span.source, "char") == 0) {
        *res = *new_type(TYPE_CHAR);
    } else {
        *res = *new_struct_type(ident->span.source, NULL, 0, new_method_list(), NULL);
    }
    if (is_ptr) {
        return *new_ptr_type(res);
    }
    return *res;
}

Stmt *parse_func(Parser *parser) {
    Token *func_token = parser->tokens->tokens[parser->current];
    parser->current++;
    GenericList *generic = new_generic_list();
    if (check(parser, TOKEN_LT)) {
        parser->current++;
        while (!check(parser, TOKEN_GT)) {
            if (generic->size > 0) {
                if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                    add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                    return NULL;
                }
                parser->current++;
            }
            Type ty = parse_type(parser);
            if (ty.kind == TYPE_INVALID) {
                return NULL;
            }
            append_generic(generic, &ty);
        }
        if (parser->tokens->tokens[parser->current]->type != TOKEN_GT) {
            add_error(errorList, "Expected '>'", func_token->span, parser->source);
            return NULL;
        }
        parser->current++;
    }
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", func_token->span, parser->source);
        return NULL;
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Param *args = malloc(sizeof(Param));
    int num_params = 0;
    while (!check(parser, TOKEN_RPAREN)) {
        if (num_params > 0) {
            if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                return NULL;
            }
            parser->current++;
        }
        Param param = parse_param(parser);
        if (param.name == NULL) {
            return NULL;
        }
        args = realloc(args, sizeof(Param) * (num_params + 1));
        args[num_params] = param;
        num_params++;
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
        add_error(errorList, "Expected ')'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    if (parser->tokens->tokens[parser->current]->type != TOKEN_COL) {
        add_error(errorList, "Expected ':'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Type ret = parse_type(parser);
    if (ret.kind == TYPE_INVALID) {
        return NULL;
    }
    Stmt *body = parse_stmt(parser);
    if (body == NULL) {
        return NULL;
    }
    return new_function_stmt(ident->span.source, generic, args, num_params, body,
                             extend_span(parser->source, func_token->span, body->span), ret);


}

Stmt *parse_if(Parser *parser) {
    Token *if_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", if_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *cond = parse_expr(parser);
    if (cond == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_RPAREN)) {
        add_error(errorList, "Expected ')'", if_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Stmt *body = parse_stmt(parser);
    if (body == NULL) {
        return NULL;
    }
    if (check(parser, TOKEN_ELSE)) {
        parser->current++;
        Stmt *else_body = parse_stmt(parser);
        if (else_body == NULL) {
            return NULL;
        }
        return new_if_stmt(cond, body, extend_span(parser->source, if_token->span, else_body->span), else_body);
    }
    return new_if_stmt(cond, body, extend_span(parser->source, if_token->span, body->span), NULL);
}

Stmt *parse_while(Parser *parser) {
    Token *while_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", while_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *cond = parse_expr(parser);
    if (cond == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_RPAREN)) {
        add_error(errorList, "Expected ')'", while_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Stmt *body = parse_stmt(parser);
    if (body == NULL) {
        return NULL;
    }
    return new_while_stmt(cond, body, extend_span(parser->source, while_token->span, body->span));
}

Stmt *parse_for(Parser *parser) {
    Token *for_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", for_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *init = parse_expr(parser);
    if (init == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", for_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *cond = parse_expr(parser);
    if (cond == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", for_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *update = parse_expr(parser);
    if (update == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_RPAREN)) {
        add_error(errorList, "Expected ')'", for_token->span, parser->source);
        return NULL;
    }
    parser->current++;
    Stmt *body = parse_stmt(parser);
    if (body == NULL) {
        return NULL;
    }
    return new_for_stmt(init, cond, update, body, extend_span(parser->source, for_token->span, body->span));
}

Stmt *parse_block(Parser *parser) {
    Token *lbrace = parser->tokens->tokens[parser->current];
    parser->current++;
    StmtList *stmts = new_stmt_list();
    while (!check(parser, TOKEN_RBRACE)) {
        Stmt *stmt = parse_stmt(parser);
        if (stmt == NULL) {
            return NULL;
        }
        append_stmt(stmts, stmt);
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACE) {
        add_error(errorList, "Expected '}'", lbrace->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_block_stmt(stmts, extend_span(parser->source, lbrace->span, lbrace->span));
}

Stmt *parse_let(Parser *parser) {
    Token *let_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (parser->tokens->tokens[parser->current]->type != TOKEN_IDENTIFIER) {
        add_error(errorList, "Expected identifier", let_token->span, parser->source);
        return NULL;
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (check(parser, TOKEN_COL)) {
        parser->current++;
        Type ty = parse_type(parser);
        if (ty.kind == TYPE_INVALID) {
            return NULL;
        }
        if (!check(parser, TOKEN_ASSIGN)) {
            add_error(errorList, "Expected '='", parser->tokens->tokens[parser->current]->span, parser->source);
            return NULL;
        }
        parser->current++;
        Expr *expr = parse_expr(parser);
        if (expr == NULL) {
            return NULL;
        }
        if (!check(parser, TOKEN_SEMICOLON)) {
            add_error(errorList, "Expected ';'", expr->span, parser->source);
            return NULL;
        }
        parser->current++;
        return new_let_stmt(ident->span.source, &ty, extend_span(parser->source, let_token->span, expr->span), expr);

    }
    if (!check(parser, TOKEN_ASSIGN)) {
        add_error(errorList, "Expected '='", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Expr *expr = parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", expr->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_let_stmt(ident->span.source, NULL, extend_span(parser->source, let_token->span, expr->span), expr);
}

Stmt *parse_extern(Parser *parser) {
    Token *extern_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", extern_token->span, parser->source);
        return NULL;
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_LPAREN)) {
        add_error(errorList, "Expected '('", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Param *args = malloc(sizeof(Param));
    int num_params = 0;
    int is_vararg = 0;
    while (!check(parser, TOKEN_RPAREN)) {
        if (num_params > 0) {
            if (parser->tokens->tokens[parser->current]->type != TOKEN_COMMA) {
                add_error(errorList, "Expected ','", parser->tokens->tokens[parser->current]->span, parser->source);
                return NULL;
            }
            parser->current++;
        }
        if (check(parser, TOKEN_3DOT)) {
            parser->current++;
            is_vararg = 1;
            break;
        }
        Param param = parse_param(parser);
        if (param.name == NULL) {
            return NULL;
        }
        args = realloc(args, sizeof(Param) * (num_params + 1));
        args[num_params] = param;
        num_params++;
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RPAREN) {
        add_error(errorList, "Expected ')'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    if (parser->tokens->tokens[parser->current]->type != TOKEN_COL) {
        add_error(errorList, "Expected ':'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    Type ret = parse_type(parser);
    if (ret.kind == TYPE_INVALID) {
        return NULL;
    }
    return new_extern_stmt(ident->span.source, args, num_params, ret, is_vararg,
                           extend_span(parser->source, extern_token->span,
                                       parser->tokens->tokens[parser->current - 1]->span));
}

Stmt *parse_import(Parser *parser) {
    Token *import_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", import_token->span, parser->source);
        return NULL;
    }
    Token *path = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_SEMICOLON)) {
        add_error(errorList, "Expected ';'", path->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_import_stmt(path->span.source, extend_span(parser->source, import_token->span, path->span));
}


Stmt *parse_module(Parser *parser) {
    Token *module_token = parser->tokens->tokens[parser->current];
    parser->current++;
    if (!check(parser, TOKEN_IDENTIFIER)) {
        add_error(errorList, "Expected identifier", module_token->span, parser->source);
        return NULL;
    }
    Token *ident = parser->tokens->tokens[parser->current];
    parser->current++;
    if (check(parser, TOKEN_SEMICOLON)) {
        parser->current++;
        return new_module_stmt(ident->span.source, new_stmt_list(), extend_span(parser->source, module_token->span,
                                                                                 parser->tokens->tokens[parser->current - 1]->span));
    }
    if (!check(parser, TOKEN_LBRACE)) {
        add_error(errorList, "Expected '{'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    StmtList *stmts = new_stmt_list();
    while (!check(parser, TOKEN_RBRACE)) {
        Stmt *stmt = parse_stmt(parser);
        if (stmt == NULL) {
            return NULL;
        }
        append_stmt(stmts, stmt);
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACE) {
        add_error(errorList, "Expected '}'", ident->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_module_stmt(ident->span.source, stmts, extend_span(parser->source, module_token->span,
                                                                 parser->tokens->tokens[parser->current - 1]->span));
}

Stmt *parse_extension(Parser *parser) {
    Token *extension_token = parser->tokens->tokens[parser->current];
    parser->current++;
    Type ty = parse_type(parser);
    if (ty.kind == TYPE_INVALID) {
        return NULL;
    }
    if (!check(parser, TOKEN_LBRACE)) {
        add_error(errorList, "Expected '{'", parser->tokens->tokens[parser->current]->span, parser->source);
        return NULL;
    }

    parser->current++;
    MethodList *methods = new_method_list();
    while (!check(parser, TOKEN_RBRACE)) {
        Method *method = parse_method(parser);
        if (method == NULL) {
            return NULL;
        }
        append_method(methods, method);
    }
    if (parser->tokens->tokens[parser->current]->type != TOKEN_RBRACE) {
        add_error(errorList, "Expected '}'", parser->tokens->tokens[parser->current]->span, parser->source);
        return NULL;
    }
    parser->current++;
    return new_extension_stmt(new_extension(ty, methods), extend_span(parser->source, extension_token->span, parser->tokens->tokens[parser->current - 1]->span));


}

StmtList *parse(const char *source, const char *file) {
    TokenList *tokens = tokenize(source, file);
    if (tokens == NULL) {
        return NULL;
    }
    Parser *parser = malloc(sizeof(Parser));
    if (parser == NULL) {
        return NULL;
    }
    init_parser(parser, new_string(source), tokens);
    StmtList *stmts = new_stmt_list();
    while (!is_eof(parser)) {
        Stmt *stmt = parse_stmt(parser);
        if (stmt == NULL) {
            free_parser(parser);
            return NULL;
        }
        append_stmt(stmts, stmt);
    }
    free_parser(parser);
    return stmts;
}