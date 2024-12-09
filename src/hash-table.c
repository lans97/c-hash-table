#include <hash-table.h>

#include <errno.h>
#include <logger.h>
#include <math.h>
#include <sds.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_KEY_LENGTH 1024

// Chose 64 bits if available, else use 32...
// Yes, I'm ignoring other architectures
#ifdef INT64_MAX
#define FNV_PRIME 1099511628211
#define FNV_OFFSET_BASIS 14695981039346656037
#else
#define FNV_PRIME 16777619
#define FNV_OFFSET_BASIS 2166136261
#endif

int table_init(table* self, size_t capacity) {
    self->capacity = capacity;
    self->elements = 0;
    self->arr = calloc(self->capacity, sizeof(pair));
    if (self->arr == NULL) {
        LOG_ERROR("Could not allocate memmory for hash table: %s",
                  strerror(errno));
        return -1;
    }
    return 0;
}

void table_deinit(table* self) {
    for (int i = 0; i < self->capacity; i++) {
        if (self->arr[i].key != NULL) {
            sdsfree(self->arr[i].key);
            sdsfree(self->arr[i].value);
        }
    }
    free(self->arr);
    self->arr = NULL;
    self->capacity = 0;
    self->elements = 0;
}

table* table_new(size_t capacity) {
    table* new_table = malloc(sizeof(table));
    if (table_init(new_table, capacity) == -1)
        return NULL;

    return new_table;
}

void table_delete(table* self) {
    table_deinit(self);
    free(self);
}

int table_insert(table* self, const char* key, const char* value) {
    size_t position = table_hash_f(key) % self->capacity;
    if (((float)self->elements + 1)/(self->capacity) > 0.85) {
        table_grow(self);
    }

    if (self->arr[position].key != NULL) {
        LOG_WARNING("Colision: <%s, %s>", self->arr[position].key,
                    self->arr[position].value);
        return -1;
    }

    self->arr[position].key = sdsnew(key);
    self->arr[position].value = sdsnew(value);

    self->elements++;
    return 0;
}

sds table_get(table* self, const char* key) {
    size_t position = table_hash_f(key) % self->capacity;

    if (self->arr[position].key == NULL ||
        strcmp(self->arr[position].key, key) != 0) {
        LOG_INFO("No entry with key '%s'", key);
        return NULL;
    }

    return self->arr[position].value;
}

int table_remove(table* self, const char* key) {
    size_t position = table_hash_f(key) % self->capacity;

    if (self->arr[position].key == NULL ||
        strcmp(self->arr[position].key, key) != 0) {
        LOG_INFO("No entry with key '%s'", key);
        return -1;
    }

    sdsfree(self->arr[position].key);
    sdsfree(self->arr[position].value);
    self->arr[position].key = NULL;
    self->arr[position].value = NULL;

    self->elements--;
    return 0;
}

int table_grow(table* self) {
    LOG_DEBUG("Table is growing");
    size_t new_capacity = self->capacity * 2;
    while (!primality_division(new_capacity, new_capacity)) {
        new_capacity++;
    }

    pair* new_arr = calloc(new_capacity, sizeof(pair));
    size_t new_position;

    for (int i = 0; i < self->capacity; i++) {
        if (self->arr[i].key != NULL) {
            new_position = table_hash_f(self->arr[i].key) % new_capacity;

            if (new_arr[new_position].key != NULL) {
                LOG_WARNING("Colision: <%s, %s>", new_arr[new_position].key,
                            new_arr[new_position].value);
                continue;
            }
            new_arr[new_position].key = self->arr[i].key;
            new_arr[new_position].value = self->arr[i].value;
        }
    }

    free(self->arr);
    self->arr = new_arr;
    self->capacity = new_capacity;

    return 0;
}

size_t table_hash_f(const char* str) {
    // FNV-1a implementation
    size_t hash = FNV_OFFSET_BASIS;
    for (int i = 0; str[i] != 0 && i < MAX_KEY_LENGTH; i++) {
        hash = hash ^ str[i];
        hash = hash * FNV_PRIME;
    }

    return hash;
}

bool primality_division(unsigned int n, unsigned int limit) {
    for (int i = 2; i < limit; i++) {
        if (n % i == 0)
            return false;
    }
    return true;
}
