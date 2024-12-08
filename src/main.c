#include <stdio.h>
#include <sds.h>
#include <hash-table.h>

int main(int argc, char* argv[]) {
    sds s1 = sdsnew("Hello, world!");
    sds s2 = sdsnew("Test string");
    printf("s1: %s\n", s1);
    size_t s1_hash = table_hash_f(s1, sdslen(s1));
    printf("s1_hash: %zu\n", s1_hash);

    printf("s2: %s\n", s2);
    size_t s2_hash = table_hash_f(s2, sdslen(s2));
    printf("s1_hash: %zu\n", s1_hash);

    sdsfree(s1);
    sdsfree(s2);
    return 0;
}
