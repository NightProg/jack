#include "collection.h"
#include <stdlib.h>
#include <string.h>

Map *new_map() {
    Map *map = malloc(sizeof(Map));
    if (map == NULL) {
        return NULL;
    }
    map->size = 0;
    map->capacity = 8;
    map->pairs = malloc(sizeof(KeyValuePair) * map->capacity);
    if (map->pairs == NULL) {
        free(map);
        return NULL;
    }
    return map;
}

void map_put(Map *map, void *key, void *value) {
    if (map->size == map->capacity) {
        map->capacity *= 2;
        KeyValuePair *new_pairs = malloc(sizeof(KeyValuePair) * map->capacity);
        if (new_pairs == NULL) {
            return;
        }
        memcpy(new_pairs, map->pairs, sizeof(KeyValuePair) * map->size);
        free(map->pairs);
        map->pairs = new_pairs;
    }
    map->pairs[map->size].key = key;
    map->pairs[map->size].value = value;
    map->size++;
}

void *map_get(Map *map, void *key, size_t key_size) {
    for (size_t i = 0; i < map->size; i++) {
        if (memcmp(map->pairs[i].key, key, key_size) == 0) {
            return map->pairs[i].value;
        }
    }
    return NULL;
}

void free_map(Map *map) {
    free(map->pairs);
    free(map);
}
