#include "lexer.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

Token *new_token(TokenType type, Span span) {
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        return NULL;
    }
    token->type = type;
    token->span = span;
    return token;
}

void free_token(Token *token) {
    free(token);
}

TokenList *new_token_list() {
    TokenList *list = malloc(sizeof(TokenList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->tokens = malloc(sizeof(Token*) * list->capacity);
    if (list->tokens == NULL) {
        free(list);
        return NULL;
    }
    return list;
}

void append_token(TokenList *list, Token *token) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Token** new_tokens = malloc(sizeof(Token*) * list->capacity);
        if (new_tokens == NULL) {
            return;
        }
        memcpy(new_tokens, list->tokens, sizeof(Token*) * list->size);
        free(list->tokens);
        list->tokens = new_tokens;
    }
    list->tokens[list->size++] = token;
}

void free_token_list(TokenList *list) {
    for (int i = 0; i < list->size; i++) {
        free_token(list->tokens[i]);
    }
    free(list->tokens);
    free(list);
}

void init_lexer(Lexer *lexer, const char *source, String *file) {
    lexer->start = new_string(source);
    lexer->file = file;
    lexer->current_pos = 0;
    lexer->line = 1;
}

Token* next_token(Lexer *lexer) {
    switch (char_at(lexer->start, lexer->current_pos)) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            while (char_at(lexer->start, lexer->current_pos) == ' ' ||
                   char_at(lexer->start, lexer->current_pos) == '\t' ||
                   char_at(lexer->start, lexer->current_pos) == '\n' ||
                   char_at(lexer->start, lexer->current_pos) == '\r') {
                if (char_at(lexer->start, lexer->current_pos) == '\n') {
                    lexer->line++;
                }
                lexer->current_pos++;
            }
            return next_token(lexer);
        case '=':
            if (char_at(lexer->start, lexer->current_pos + 1) == '=') {
                lexer->current_pos += 2;
                return new_token(TOKEN_EQ, span(
                        lexer->current_pos - 2,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 2, lexer->current_pos),
                        lexer->file
                ));
            } else {
                lexer->current_pos++;
                return new_token(TOKEN_ASSIGN, span(
                        lexer->current_pos - 1,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                        lexer->file
                ));
            }
        case '<':
            if (char_at(lexer->start, lexer->current_pos + 1) == '=') {
                lexer->current_pos += 2;
                return new_token(TOKEN_LTE, span(
                        lexer->current_pos - 2,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 2, lexer->current_pos),
                        lexer->file
                ));
            } else {
                lexer->current_pos++;
                return new_token(TOKEN_LT, span(
                        lexer->current_pos - 1,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                        lexer->file
                ));
            }
        case '>':
            if (char_at(lexer->start, lexer->current_pos + 1) == '=') {
                lexer->current_pos += 2;
                return new_token(TOKEN_GTE, span(
                        lexer->current_pos - 2,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 2, lexer->current_pos),
                        lexer->file
                ));
            } else {
                lexer->current_pos++;
                return new_token(TOKEN_GT, span(
                        lexer->current_pos - 1,
                        lexer->current_pos,
                        lexer->line,
                        slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                        lexer->file
                ));
            }
        case '(':
            lexer->current_pos++;
            return new_token(TOKEN_LPAREN,
                             span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                     )
                             );
        case ')':
            lexer->current_pos++;
            return new_token(TOKEN_RPAREN, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '{':
            lexer->current_pos++;
            return new_token(TOKEN_LBRACE, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '}':
            lexer->current_pos++;
            return new_token(TOKEN_RBRACE, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '[':
            lexer->current_pos++;
            return new_token(TOKEN_LBRACKET, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case ']':
            lexer->current_pos++;
            return new_token(TOKEN_RBRACKET, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '\'':
            lexer->current_pos++;
            char c = char_at(lexer->start, lexer->current_pos);
            if (c == '\\') {
                lexer->current_pos++;
                c = char_at(lexer->start, lexer->current_pos);
                if (c == 'n') {
                    c = '\n';
                } else if (c == 't') {
                    c = '\t';
                } else if (c == 'r') {
                    c = '\r';
                } else if (c == '\\') {
                    c = '\\';
                } else if (c == '\'') {
                    c = '\'';
                } else if (c == '0') {
                    c = '\0';
                } else {
                    return NULL;
                }
            }
            lexer->current_pos++;
            if (char_at(lexer->start, lexer->current_pos) != '\'') {
                return NULL;
            }
            lexer->current_pos++;
            return new_token(TOKEN_CHAR, span(
                                     lexer->current_pos - 3,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 3, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case ',':
            lexer->current_pos++;
            return new_token(TOKEN_COMMA, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case ';':
            lexer->current_pos++;
            return new_token(TOKEN_SEMICOLON, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '.':
            lexer->current_pos++;
            if (char_at(lexer->start, lexer->current_pos) == '.' && char_at(lexer->start, lexer->current_pos + 1) == '.') {
                lexer->current_pos += 2;
                return new_token(TOKEN_3DOT, span(
                                         lexer->current_pos - 3,
                                         lexer->current_pos,
                                         lexer->line,
                                         slice_string(lexer->start, lexer->current_pos - 3, lexer->current_pos),
                                         lexer->file
                                    )
                                 );
            }
            return new_token(TOKEN_DOT, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case ':':
            lexer->current_pos++;
            if (char_at(lexer->start, lexer->current_pos) == ':') {
                lexer->current_pos++;
                return new_token(TOKEN_DOUBLE_COLON, span(
                                         lexer->current_pos - 2,
                                         lexer->current_pos,
                                         lexer->line,
                                         slice_string(lexer->start, lexer->current_pos - 2, lexer->current_pos),
                                         lexer->file
                                    )
                                 );
            }
            return new_token(TOKEN_COL, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '+':
            lexer->current_pos++;
            return new_token(TOKEN_PLUS, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '-':
            lexer->current_pos++;
            if (char_at(lexer->start, lexer->current_pos) == '>') {
                lexer->current_pos++;
                return new_token(TOKEN_ARROW, span(
                                         lexer->current_pos - 2,
                                         lexer->current_pos,
                                         lexer->line,
                                         slice_string(lexer->start, lexer->current_pos - 2, lexer->current_pos),
                                         lexer->file
                                    )
                                 );
            }
            return new_token(TOKEN_MINUS, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '*':
            lexer->current_pos++;
            return new_token(TOKEN_MUL, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '/':
            lexer->current_pos++;
            return new_token(TOKEN_DIV, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '&':
            lexer->current_pos++;
            return new_token(TOKEN_AMP, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '!':
            lexer->current_pos++;
            return new_token(TOKEN_NOT, span(
                                     lexer->current_pos - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, lexer->current_pos - 1, lexer->current_pos),
                                     lexer->file
                                )
                             );
        case '\0':
            return new_token(TOKEN_EOF, span(
                                     lexer->current_pos,
                                     lexer->current_pos,
                                     lexer->line,
                                     new_string(""),
                                     lexer->file
                                )
                             );
        case '\"': {
            lexer->current_pos++;
            int start = lexer->current_pos;
            while (char_at(lexer->start, lexer->current_pos) != '\"') {
                if (char_at(lexer->start, lexer->current_pos) == '\0') {
                    return NULL;
                }
                lexer->current_pos++;
            }
            String *value = slice_string(lexer->start, start, lexer->current_pos);
            lexer->current_pos++;
            return new_token(TOKEN_STRING, span(
                                     start - 1,
                                     lexer->current_pos,
                                     lexer->line,
                                     value,
                                     lexer->file
                                )
                             );
        }
        default: {
            char c = char_at(lexer->start, lexer->current_pos);
            if (isdigit(c)) {
                int start = lexer->current_pos;
                while (isdigit(char_at(lexer->start, lexer->current_pos))) {
                    lexer->current_pos++;
                }

                return new_token(TOKEN_NUMBER, span(
                                     start,
                                     lexer->current_pos,
                                     lexer->line,
                                     slice_string(lexer->start, start, lexer->current_pos),
                                     lexer->file
                                )
                             );
            } else if (isalpha(c) || c == '_') {
                int start = lexer->current_pos;
                while (isalnum(char_at(lexer->start, lexer->current_pos)) || char_at(lexer->start, lexer->current_pos) == '_') {
                    lexer->current_pos++;
                }

                String *value = slice_string(lexer->start, start, lexer->current_pos);
                if (compare_string(value, "if") == 0) {
                    return new_token(TOKEN_IF, span(
                                        start,
                                        lexer->current_pos,
                                        lexer->line,
                                        value,
                                        lexer->file
                            ));
                } else if (compare_string(value, "else") == 0) {
                    return new_token(TOKEN_ELSE, span(
                                        start,
                                        lexer->current_pos,
                                        lexer->line,
                                        value,
                                        lexer->file
                            ));
                } else if (compare_string(value, "while") == 0) {
                    return new_token(TOKEN_WHILE, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "for") == 0) {
                    return new_token(TOKEN_FOR, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "return") == 0) {
                    return new_token(TOKEN_RETURN, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "let") == 0) {
                    return new_token(TOKEN_LET, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "func") == 0) {
                    return new_token(TOKEN_FUNC, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "struct") == 0) {
                    return new_token(TOKEN_STRUCT, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "extern") == 0) {
                    return new_token(TOKEN_EXTERN, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "null") == 0) {
                    return new_token(TOKEN_NULL, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "as") == 0) {
                    return new_token(TOKEN_AS, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "self") == 0) {
                    return new_token(TOKEN_SELF, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "import") == 0) {
                    return new_token(TOKEN_IMPORT, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "module") == 0) {
                    return new_token(TOKEN_MODULE, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "extension") == 0) {
                    return new_token(TOKEN_EXTENSION, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                } else if (compare_string(value, "op") == 0) {
                    return new_token(TOKEN_OP, span(
                            start,
                            lexer->current_pos,
                            lexer->line,
                            value,
                            lexer->file
                    ));
                }
                return new_token(TOKEN_IDENTIFIER, span(
                        start,
                        lexer->current_pos,
                        lexer->line,
                        value,
                        lexer->file
                ));
            } else {
                return NULL;
            }
        }

    }
}

void free_lexer(Lexer *lexer) {
    free_string(lexer->start);
    free(lexer);
}

TokenList *tokenize(const char *source, const char *file) {
    Lexer lexer;
    init_lexer(&lexer, source, new_string(file));
    TokenList *list = new_token_list();
    Token *token;
    while ((token = next_token(&lexer)) != NULL && token->type != TOKEN_EOF) {
        append_token(list, token);
    }
    return list;
}
