#include <stdio.h>
#include <sds.h>
#include <hash-table.h>
#include <logger.h>

int main(int argc, char* argv[]) {
    table t1;
    table_init(&t1, 7);

    table_insert(&t1, "test", "123");
    table_insert(&t1, "key1", "321");
    table_insert(&t1, "somethign", "blabla");
    table_insert(&t1, "fdjskal", "fdjsabvb");
    table_insert(&t1, "dfjsah", "fdsjav");

    table_print(&t1);

    printf("%s: %s\n", "fdjskal", table_get(&t1, "fdjskal"));
    printf("%s: %s\n", "somethign", table_get(&t1, "somethign"));

    table_deinit(&t1);
    return 0;
}
