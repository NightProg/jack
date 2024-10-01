#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Error *new_error(const char *message, Span span, String *source) {
    Error* error = malloc(sizeof(Error));
    if (error == NULL) {
        return NULL;
    }
    error->message = message;
    error->source = source;
    error->span = span;
    return error;
}

void print_error_list() {
    fprintf(stderr, "%s", error_list_to_string(errorList)->data);
}

String *error_list_to_string(ErrorList *list) {
    String *str = new_string("");
    for (int i = 0; i < list->size; i++) {
        Error *error = list->errors[i];
        String *span_str = span_to_string(error->span);
        add_c_string(str, error->message);
        add_c_string(str, " at ");
        add_string(str, span_str);
        add_c_string(str, "\n");
        add_c_string(str, get_span_line(error->source, error->span));
        add_c_string(str, "\n");
//        Span span = fix_span_by_line(error->source, error->span, error->span.line);
//        for (int j = -1; j < span.start; j++) {
//            add_char(str, ' ');
//        }
//
//        for (int j = span.start; j < span.end-1; j++) {
//            add_char(str, '^');
//        }
        add_c_string(str, "\n");


        free_string(span_str);
    }
    return str;
}


void init_error_list(ErrorList *list) {
    list->size = 0;
    list->capacity = 4;
    list->errors = malloc(sizeof(Error*) * list->capacity);
}

void add_error(ErrorList *list, const char *message, Span span, String *source) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        Error** new_errors = malloc(sizeof(Error*) * list->capacity);
        if (new_errors == NULL) {
            return;
        }
        memcpy(new_errors, list->errors, sizeof(Error*) * list->size);
        free(list->errors);
        list->errors = new_errors;
    }
    list->errors[list->size++] = new_error(message, span, source);
}



void free_error_list(ErrorList *list) {
    free(list->errors);
}