#include <hash-table.h>

#include <errno.h>
#include <logger.h>
#include <sds.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
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

pair* pair_new(const char* key, const char* value, pair* p_next) {
    pair* new_pair = malloc(sizeof(pair));
    if (new_pair == NULL) {
        LOG_ERROR("Could not allocate memory for new pair: %s",
                  strerror(errno));
        return NULL;
    }
    new_pair->key = sdsnew(key);
    new_pair->value = sdsnew(value);
    new_pair->p_next = p_next;

    return new_pair;
}

void pair_delete(pair* self) {
    sdsfree(self->key);
    sdsfree(self->value);
    free(self);
}

// Bucket interface
int bucket_init(bucket* self) {
    self->head = NULL;
    self->lenght = 0;
    return 0;
}

void bucket_deinit(bucket* self) {
    while (bucket_popFront(self) != -1)
        ;
}

bucket* bucket_new() {
    bucket* new_bucket = malloc(sizeof(bucket));
    if (new_bucket == NULL) {
        LOG_ERROR("Could not allocate memory for new bucket: %s",
                  strerror(errno));
        return NULL;
    }

    bucket_init(new_bucket);
    return new_bucket;
}

void bucket_delete(bucket* self) {
    bucket_deinit(self);
    free(self);
    self = NULL;
}

int bucket_insert(bucket* self, const char* key, const char* value) {
    pair* new_pair = pair_new(key, value, self->head);
    if (new_pair == NULL)
        return -1;

    self->head = new_pair;
    self->lenght++;
    return 0;
}

int bucket_append(bucket* self, const char* key, const char* value) {
    pair* new_pair = pair_new(key, value, NULL);
    if (new_pair == NULL)
        return -1;

    if (self->head == NULL) {
        self->head = new_pair;
        self->lenght++;
        return 0;
    }

    pair* curr_pair = self->head;
    while (curr_pair->p_next != NULL)
        curr_pair = curr_pair->p_next;

    curr_pair->p_next = new_pair;
    self->lenght++;

    return 0;
}

int bucket_popBack(bucket* self) {
    if (self->head == NULL) {
        LOG_DEBUG("Bucket is already empty...");
        return -1;
    }

    if (self->head->p_next == NULL) {
        pair_delete(self->head);
        self->head = NULL;
        return 0;
    }

    pair* curr_pair = self->head;
    while (curr_pair->p_next->p_next != NULL)
        curr_pair = curr_pair->p_next;

    pair_delete(curr_pair->p_next->p_next);
    curr_pair->p_next = NULL;
    self->lenght--;

    return 0;
}

int bucket_popFront(bucket* self) {
    if (self->head == NULL) {
        return -1;
    }

    pair* new_head = self->head->p_next;
    pair_delete(self->head);
    self->head = new_head;
    self->lenght--;

    return 0;
}

pair* bucket_get(bucket* self, const char* key) {
    pair* match = self->head;

    while (match != NULL && strcmp(match->key, key) != 0)
        match = match->p_next;

    return match;
}

int bucket_remove(bucket* self, const char* key) {
    pair* match = self->head;
    pair* last_pair;

    if (match != NULL && strcmp(match->key, key) == 0) {
        bucket_popFront(self);
        return 0;
    }

    while (match != NULL && strcmp(match->key, key) != 0) {
        last_pair = match;
        match = match->p_next;
    }

    if (match == NULL) {
        LOG_DEBUG("Bucket is already empty...");
        return -1;
    }

    last_pair->p_next = match->p_next;
    pair_delete(match);
    self->lenght--;
    return 0;
}

void bucket_print(bucket* self) {
    pair* curr_pair = self->head;

    while (curr_pair != NULL) {
        printf("(\"%s\",\"%s\") => ", curr_pair->key, curr_pair->value);
        curr_pair = curr_pair->p_next;
    }

    printf("null\n");
}

// Hash table interface
int table_init(table* self, size_t capacity) {
    self->capacity = capacity;
    self->elements = 0;
    self->arr = calloc(self->capacity, sizeof(pair));
    if (self->arr == NULL) {
        LOG_ERROR("Could not allocate memory for hash table: %s",
                  strerror(errno));
        return -1;
    }
    return 0;
}

void table_deinit(table* self) {
    for (int i = 0; i < self->capacity; i++) {
        if (self->arr[i].lenght != 0)
            bucket_deinit(&self->arr[i]);
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
    if (((float)self->elements + 1) / (self->capacity) > 0.85) {
        table_grow(self);
    }

    pair* match = bucket_get(&self->arr[position], key);
    if (match != NULL) {
        match->value = sdscpy(match->value, value);
        return 0;
    }

    bucket_append(&self->arr[position], key, value);

    self->elements++;
    return 0;
}

sds table_get(table* self, const char* key) {
    size_t position = table_hash_f(key) % self->capacity;

    pair* match = bucket_get(&self->arr[position], key);

    if (match == NULL)
        return NULL;
    else
        return match->value;
}

int table_remove(table* self, const char* key) {
    size_t position = table_hash_f(key) % self->capacity;

    if (bucket_remove(&self->arr[position], key) != 0)
        return -1;

    self->elements--;
    return 0;
}

int table_grow(table* self) {
    size_t new_capacity = self->capacity * 2;
    while (!primality_division(new_capacity, new_capacity)) {
        new_capacity++;
    }

    bucket* new_arr = calloc(new_capacity, sizeof(bucket));
    size_t new_position;

    for (int i = 0; i < self->capacity; i++) {
        while (self->arr[i].lenght != 0) {
            new_position = table_hash_f(self->arr[i].head->key) % new_capacity;

            bucket_append(&new_arr[new_position], self->arr[i].head->key,
                          self->arr[i].head->value);
            bucket_popFront(&self->arr[i]);
        }
    }

    free(self->arr);
    self->arr = new_arr;
    self->capacity = new_capacity;

    return 0;
}

void table_print(table* self) {
    for (size_t i = 0; i < self->capacity; i++) {
        printf("%02zu: ", i);
        bucket_print(&self->arr[i]);
    }
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
