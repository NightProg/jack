#ifndef JACK_PARSER_H
#define JACK_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "error.h"
#include "gc.h"



typedef struct {
    TokenList *tokens;
    String *source;
    size_t current;
} Parser;

void init_parser(Parser *parser, String *source, TokenList *tokens);
void free_parser(Parser *parser);
int is_eof(Parser *parser);
int check(Parser *parser, TokenType type);
Type parse_type(Parser *parser);
Expr *parse_expr(Parser *parser);
Expr *parse_ref(Parser *parser);
Expr *parse_deref(Parser *parser);
Expr *parse_unary(Parser *parser);
Expr *parse_primary(Parser *parser);
Expr *parse_term(Parser *parser);
Expr *parse_factor(Parser *parser);
Expr *parse_cmp(Parser *parser);
Expr *parse_index(Parser *parser);
Expr *parse_as(Parser *parser);
Expr *parse_call(Parser *parser);
Expr *parse_assign(Parser *parser);
Expr *parse_get(Parser *parser);
Expr *parse_init(Parser *parser);

Stmt *parse_struct(Parser *parser);
Stmt *parse_return(Parser *parser);
Stmt *parse_func(Parser *parser);
Stmt *parse_stmt(Parser *parser);
Stmt *parse_if(Parser *parser);
Stmt *parse_while(Parser *parser);
Stmt *parse_for(Parser *parser);
Stmt *parse_block(Parser *parser);
Stmt *parse_let(Parser *parser);
Stmt *parse_extern(Parser *parser);
Stmt *parse_import(Parser *parser);
Stmt *parse_module(Parser *parser);
Stmt *parse_extension(Parser *parser);

Method *parse_method(Parser *parser);
OpOverload *parse_op_overload(Parser *parser);


StmtList *parse(const char *source, const char *file);


#endif //JACK_PARSER_H
