#include "sds.h"
#include <hash-table.h>
#include <stddef.h>
#include <stdlib.h>
#include <logger.h>

#define FNV32_PRIME 16777619;
#define FNV32_OFFSET_BASIS 2166136261;

#define FNV64_PRIME 1099511628211;
#define FNV64_OFFSET_BASIS 14695981039346656037;

int table_init(table* self, size_t capacity) {
    self->capacity = capacity;
    self->elements = 0;
    self->arr = (pair*) calloc(self->capacity, sizeof(pair));
    return 0;
}

void table_deinit(table* self) {
    free(self->arr);
    self->arr = NULL;
    self->capacity = 0;
    self->elements = 0;
}

table* table_new(size_t capacity) {
    table* new_table = (table*) malloc(sizeof(table));
    table_init(new_table, capacity);

    return new_table;
}

void table_delete(table* self) {
    table_deinit(self);
    free(self);
}

int table_insert(const char* key, const char* value) {
    return 0;
}

int table_get(const char* key) {
    return 0;
}

int table_remove(const char* key) {
    return 0;
}

size_t table_hash_f(const char* str, size_t length) {
    if (length <= 0) {
        LOG_ERROR("Length must be greater than 0");
        return 0;
    }
    char prehash[4];
    bool lgt1 = (length > 1);

    prehash[0] = str[0];
    prehash[1] = str[1] * lgt1;
    prehash[2] = str[length - 2] * lgt1;
    prehash[3] = str[length - 1] * lgt1;

    // FNV-1a implementation (32 bits)
    size_t hash = FNV32_OFFSET_BASIS;
    for (int i = 0; i < 4; i++) {
        hash = hash ^ prehash[i];
        hash = hash * FNV32_PRIME;
    }

    return hash;
}
