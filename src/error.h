#ifndef JACK_ERROR_H
#define JACK_ERROR_H

#include "span.h"
#include "string.h"
#include "gc.h"
#include <stddef.h>


typedef struct {
    Span span;
    const char *message;
    String *source;
} Error;

typedef struct {
    Error **errors;
    size_t size;
    size_t capacity;
} ErrorList;

void init_error_list(ErrorList *list);
void free_error_list(ErrorList *list);
void add_error(ErrorList *list, const char *message, Span span, String *source);

void print_error_list();
String *error_list_to_string(ErrorList *list);

ErrorList* errorList;

#endif //JACK_ERROR_H
