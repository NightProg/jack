#ifndef JACK_SPAN_H
#define JACK_SPAN_H

#include "string.h"
#include "gc.h"

typedef struct {
    int start;
    int end;
    int line;
    String *source;
    String *file;
} Span;

Span span(int start, int end, int line, String *source, String *file);
char* get_span_line(String *source, Span span);
Span fix_span_by_line(String* source, Span span, int line);
String *string_from_span(String *source, Span span);
String *span_to_string(Span span);
String *sum_string_from_span(Span *s, int n);
Span extend_span(String *total, Span s0, Span s1);

#endif //JACK_SPAN_H
