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

int table_insert(const char* key, const char* value);
int table_get(const char* key);
int table_remove(const char* key);

size_t table_hash_f(const char* str, size_t length);

#endif
