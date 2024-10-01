#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

String* new_string(const char* data) {
    String* string = malloc(sizeof(String));
    if (string == NULL) {
        return NULL;
    }
    string->data = strdup(data);
    string->length = strlen(data);
    return string;
}

int add_char(String* string, char c) {
    size_t len = strlen(string->data);
    string->data = realloc(string->data, len + 2);
    if (string->data == NULL) {
        return -1;
    }
    string->data[len] = c;
    string->data[len + 1] = '\0';
    string->length++;
    return 0;
}

int add_c_string(String* string, const char* data) {
    size_t len = strlen(string->data);
    size_t data_len = strlen(data);
    string->data = realloc(string->data, len + data_len + 1);
    if (string->data == NULL) {
        return -1;
    }
    strncpy(string->data + len, data, data_len);
    string->length += data_len;
    return 0;
}

int add_string(String* string, String* other) {
    return add_c_string(string, other->data);
}

int sprintf_string(String* string, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (len < 0) {
        return -1;
    }
    char* buffer = malloc(len + 1);
    if (buffer == NULL) {
        return -1;
    }
    va_start(args, format);
    vsnprintf(buffer, len + 1, format, args);
    va_end(args);
    add_c_string(string, buffer);
    free(buffer);
    return 0;
}

int compare_string(String* string, const char* data) {
    return strcmp(string->data, data);
}

int compare_strings(String* string, String* other) {
    return strcmp(string->data, other->data);
}

String* slice_string(String* string, int start, int end) {
    if (start < 0 || start >= string->length || end < 0 || end > string->length || start > end) {
        return NULL;
    }
    if (end == string->length) {
        return new_string(string->data + start);
    }
    String* result = new_string(string->data + start);
    result->length = end - start;
    result->data[end - start] = '\0';
    return result;
}

char char_at(String* string, int index) {
    if (index < 0 || index >= string->length) {
        return '\0';
    }
    return string->data[index];
}

void free_string(String* string) {
    free(string->data);
    free(string);
}

StringList* new_string_list() {
    StringList* list = malloc(sizeof(StringList));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->capacity = 4;
    list->strings = malloc(list->capacity * sizeof(String*));
    if (list->strings == NULL) {
        free(list);
        return NULL;
    }
    return list;
}

int remove_string(StringList* list, int index) {
    if (index < 0 || index >= list->size) {
        return -1;
    }
    for (int i = index; i < list->size - 1; i++) {
        list->strings[i] = list->strings[i + 1];
    }
    list->size--;
    return 0;
}

void append_string(StringList* list, String* string) {
    if (list->size == list->capacity) {
        list->capacity *= 2;
        list->strings = realloc(list->strings, list->capacity * sizeof(String*));
        if (list->strings == NULL) {
            return;
        }
    }
    list->strings[list->size++] = string;
}

StringList* string_list_from_array(int length, char** array) {
    StringList* list = new_string_list();
    for (int i = 0; i < length; i++) {
        append_string(list, new_string(array[i]));
    }
    return list;
}

String* fake_ws_to_real_ws(String* s) {
    String* result = new_string("");
    for (int i = 0; i < s->length; i++) {
        if (s->data[i] == '\\') {
            if (s->data[i + 1] == 'n') {
                add_char(result, '\n');
                i++;
            } else if (s->data[i + 1] == 't') {
                add_char(result, '\t');
                i++;
            } else {
                add_char(result, s->data[i]);
            }
        } else {
            add_char(result, s->data[i]);
        }
    }
    return result;
}


void free_string_list(StringList* list) {
    for (int i = 0; i < list->size; i++) {
        free_string(list->strings[i]);
    }
    free(list->strings);
    free(list);
}