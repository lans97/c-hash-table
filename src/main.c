#include <stdio.h>
#include <sds.h>
#include <hash-table.h>

int main(int argc, char* argv[]) {
    table t1;
    table_init(&t1, 7);

    table_insert(&t1, "test", "123");
    printf("Table load: %f\n", table_load_index(&t1));
    table_insert(&t1, "key1", "321");
    printf("Table load: %f\n", table_load_index(&t1));
    table_insert(&t1, "somethign", "blabla");
    printf("Table load: %f\n", table_load_index(&t1));

    table_remove(&t1, "test");
    printf("Table load: %f\n", table_load_index(&t1));

    printf("Lookup key '%s': %s\n", "test", table_get(&t1, "test"));
    printf("Lookup key '%s': %s\n", "key1", table_get(&t1, "key1"));
    printf("Lookup key '%s': %s\n", "somethign", table_get(&t1, "somethign"));

    table_deinit(&t1);
    return 0;
}
