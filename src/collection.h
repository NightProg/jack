#ifndef JACK_COLLECTION_H
#define JACK_COLLECTION_H

#include <stddef.h>

typedef struct {
    void *key;
    void *value;
} KeyValuePair;

typedef struct {
    KeyValuePair *pairs;
    size_t size;
    size_t capacity;
} Map;

Map *new_map();
void map_put(Map *map, void *key, void *value);
void *map_get(Map *map, void *key, size_t key_size);
void free_map(Map *map);

#endif //JACK_COLLECTION_H
