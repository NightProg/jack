#define NO_GC
#include "gc.h"
#undef NO_GC
#include <stdio.h>


void gc_init() {
    global_gc = malloc(sizeof(GC));
    global_gc->head = NULL;
    global_gc->size = 0;
}

void* gc_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        return NULL;
    }
    GCMemBlock* block = malloc(sizeof(GCMemBlock));
    if (block == NULL) {
        return NULL;
    }
    block->ptr = ptr;
    block->size = size;
    block->next = global_gc->head;
    global_gc->head = block;
    global_gc->size += size;
    return ptr;
}

void gc_dump() {
    printf("GC size: %lu\n", global_gc->size);
    GCMemBlock* block = global_gc->head;
    while (block != NULL) {
        printf("Block at %p, size %lu\n", block->ptr, block->size);
        block = block->next;
    }
}

void gc_collect() {
    GCMemBlock* block = global_gc->head;
    while (block != NULL) {
        GCMemBlock* next = block->next;
        free(block->ptr);
        free(block);
        block = next;
    }
    global_gc->head = NULL;
    global_gc->size = 0;
}
