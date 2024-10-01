
#ifndef JACK_STRING_H
#define JACK_STRING_H

#include <stdlib.h>

typedef struct {
    char* data;
    size_t length;
} String;

String* new_string(const char* data);
int add_char(String* string, char c);
int add_c_string(String* string, const char* data);
int add_string(String* string, String* other);
int sprintf_string(String* string, const char* format, ...);
int compare_string(String* string, const char* data);
int compare_strings(String* string, String* other);
String* slice_string(String* string, int start, int end);
char char_at(String* string, int index);
void free_string(String* string);
String* fake_ws_to_real_ws(String* s);

typedef struct {
    int size;
    int capacity;
    String** strings;
} StringList;

StringList* new_string_list();
int remove_string(StringList* list, int index);
void append_string(StringList* list, String* string);
StringList* string_list_from_array(int length, char** array);
void free_string_list(StringList* list);

#endif //JACK_STRING_H
