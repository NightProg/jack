#ifndef JACK_GC_H
#define JACK_GC_H

#include <stddef.h>
#include <stdlib.h>

typedef struct {
    void* ptr;
    size_t size;
    struct GCMemBlock* next;
} GCMemBlock;


typedef struct {
    GCMemBlock* head;
    size_t size;
} GC;

GC* global_gc;

void gc_init();

void* gc_malloc(size_t size);
void gc_dump();
void gc_collect();

#ifndef NO_GC
#define malloc gc_malloc
#endif


#endif //JACK_GC_H
