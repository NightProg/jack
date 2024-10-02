#define NO_GC
#include "gc.h"
#undef NO_GC
#include <stdio.h>


void gc_init() {
    global_gc = malloc(sizeof(GC));
    global_gc->head = NULL;
    global_gc->size = 0;
}

void *gc_malloc(size_t size, int line, int col, const char *file) {
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
    block->from_line = line;
    block->from_col = col;
    block->from_file = file;

    global_gc->head = block;
    global_gc->size += size;
    return ptr;
}

void *gc_realloc(void* ptr, size_t size, int line, int col, const char *file) {
    GCMemBlock* block = global_gc->head;
    while (block != NULL) {
        if (block->ptr == ptr) {
            global_gc->size -= block->size;
            block->ptr = realloc(ptr, size);
            block->size = size;
            global_gc->size += size;
            return block->ptr;
        }
        block = block->next;
    }
    // add new block
    ptr = realloc(ptr, size);
    if (ptr == NULL) {
        return NULL;
    }
    block = malloc(sizeof(GCMemBlock));
    if (block == NULL) {
        return NULL;
    }
    block->ptr = ptr;
    block->size = size;
    block->next = global_gc->head;
    block->from_line = line;
    block->from_col = col;
    block->from_file = file;
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
    if (block == NULL) {
        return;
    }
    while (block != NULL) {
        GCMemBlock* next = block->next;
        global_gc->size -= block->size;
        free(block->ptr);
        free(block);
        block = next;

    }
    global_gc->head = NULL;
    global_gc->size = 0;
}

void gc_free(void* ptr) {
    GCMemBlock* block = global_gc->head;
    GCMemBlock* prev = NULL;
    while (block != NULL) {
        if (block->ptr == ptr) {
            if (prev == NULL) {
                global_gc->head = block->next;
            } else {
                prev->next = block->next;
            }
            global_gc->size -= block->size;
            free(block->ptr);
            free(block);
            return;
        }
        prev = block;
        block = block->next;
    }
}
