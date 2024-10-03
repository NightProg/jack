#ifndef JACK_GC_H
#define JACK_GC_H
#define NO_GC

#include <stddef.h>
#include <stdlib.h>

typedef struct {
    void* ptr;
    size_t size;
    int from_line;
    int from_col;
    const char* from_file;
    struct GCMemBlock* next;
} GCMemBlock;


typedef struct {
    GCMemBlock* head;
    size_t size;
} GC;

GC* global_gc;

void gc_init();

void *gc_malloc(size_t size, int line, int col, const char *file);
void *gc_realloc(void* ptr, size_t size, int line, int col, const char *file);
void gc_dump();
void gc_free(void* ptr);
void gc_collect();

#ifndef NO_GC
#define malloc(size) gc_malloc(size, __LINE__, 0, __FILE__)
#define realloc(ptr, size) gc_realloc(ptr, size, __LINE__, 0, __FILE__)
#define free gc_free
#endif


#endif //JACK_GC_H
