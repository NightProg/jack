//
// Created by antoine barbier on 20/09/2024.
//

#ifndef JACK_LEXER_H
#define JACK_LEXER_H

#include <stddef.h>
#include "string.h"
#include "span.h"
#include "gc.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_ARROW,
    TOKEN_DOT,
    TOKEN_COL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_NOT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTE,
    TOKEN_GTE,
    TOKEN_ASSIGN,
    TOKEN_3DOT,
    TOKEN_AMP,
    TOKEN_DOUBLE_COLON,

// Keywords
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_LET,
    TOKEN_FUNC,
    TOKEN_STRUCT,
    TOKEN_EXTERN,
    TOKEN_NULL,
    TOKEN_AS,
    TOKEN_SELF,
    TOKEN_IMPORT,
    TOKEN_MODULE,
    TOKEN_EXTENSION,
    TOKEN_OP,
} TokenType;


typedef struct {
    TokenType type;
    Span span;
} Token;

Token* new_token(TokenType type, Span span);
void free_token(Token *token);

typedef struct {
    size_t size;
    size_t capacity;
    Token **tokens;
} TokenList;

TokenList* new_token_list();
void append_token(TokenList *list, Token *token);
void free_token_list(TokenList *list);


typedef struct {
    String* start;
    int current_pos;
    int line;
    String *file;
} Lexer;

void init_lexer(Lexer *lexer, const char *source, String *file);
Token* next_token(Lexer *lexer);
void free_lexer(Lexer *lexer);

TokenList *tokenize(const char *source, const char *file);





#endif //JACK_LEXER_H
