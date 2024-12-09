#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <sds.h>
#include <stddef.h>

typedef struct pair_s{
    sds key;
    sds value;
    struct pair_s* p_next;
} pair;

pair* pair_new(const char* key, const char* value, pair* p_next);
void pair_delete(pair* self);

typedef struct {
    pair* head;
    unsigned int lenght;
} bucket;

int bucket_init(bucket* self);
void bucket_deinit(bucket* self);

bucket* bucket_new();
void bucket_delete(bucket* self);

int bucket_insert(bucket* self, const char* key, const char* value);
int bucket_append(bucket* self, const char* key, const char* value);
int bucket_popBack(bucket* self);
int bucket_popFront(bucket* self);

pair* bucket_get(bucket* self, const char* key);
int bucket_remove(bucket* self, const char* key);

void bucket_print(bucket* self);

typedef struct {
    bucket* arr;
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

void table_print(table* self);

size_t table_hash_f(const char* str);

bool primality_division(unsigned int n, unsigned int limit);

#endif
