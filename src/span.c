#include <string.h>
#include "span.h"
#include <stdio.h>

Span span(int start, int end, int line, String *source, String *file) {
    Span span;
    span.start = start;
    span.end = start + strlen(source->data);
    span.line = line;
    span.source = source;
    span.file = file;
    return span;
}

String *string_from_span(String *source, Span span) {
    return slice_string(source, span.start, span.end);
}

String *sum_string_from_span(Span *s, int n) {
    String *str = new_string("");
    for (int i = 0; i < n; i++) {
        add_string(str, s[i].source);
    }
    return str;
}

Span extend_span(String *total, Span s0, Span s1) {
    Span span;
    span.start = s0.start;
    span.end = s1.end;
    span.line = s0.line;
    span.source = string_from_span(total, span);
    span.file = s0.file;
    return span;
}

char* get_span_line(String *source, Span span) {
    int start = span.start;
    while (start > 0 && source->data[start] != '\n') {
        start--;
    }
    int end = span.start;
    while (end < source->length && source->data[end] != '\n') {
        end++;
    }
    return strndup(source->data + start, end - start);
}

Span fix_span_by_line(String* source, Span span, int line) {
    if (line == 1) {
        return span;
    }
    String *lexeme = string_from_span(source, span);
    int start = 0;
    int end = 0;
    int line_count = 1;
    for (int i = 0; i < lexeme->length; i++) {
        if (line_count == line) {
            start = i;
            break;
        }
        if (lexeme->data[i] == '\n') {
            line_count++;
        }
    }
    Span new_span;
    new_span.start = start;
    new_span.end = start + lexeme->length;
    new_span.line = line;
    return new_span;
}


String *span_to_string(Span span) {
    String *str = new_string("");
    sprintf_string(str, "%s:%d:%d line %d", span.file->data, span.start, span.end, span.line);
    return str;
}


