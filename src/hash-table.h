#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <sds.h>
#include <stddef.h>

typedef struct {
    sds key;
    sds value;
} pair;

typedef struct {
    pair* arr;
    size_t capacity;
    size_t elements;
} table;

int table_init(table* self, size_t capacity);
void table_deinit(table* self);

table* table_new(size_t capacity);
void table_delete(table* self);

int table_insert(table* self, const char* key, const char* value);
sds table_get(table* self, const char* key);
int table_remove(table* self, const char* key);

int table_grow(table* self);

size_t table_hash_f(const char* str);

bool primality_division(unsigned int n, unsigned int limit);

#endif
