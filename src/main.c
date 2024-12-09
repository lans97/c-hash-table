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

    printf("Hash table:\n");
    for (int i = 0; i < t1.capacity; i++) {
        printf("%d: <%s,%s>\n", i, t1.arr[i].key, t1.arr[i].value);
    }

    table_grow(&t1);

    printf("Hash table:\n");
    for (int i = 0; i < t1.capacity; i++) {
        printf("%d: <%s,%s>\n", i, t1.arr[i].key, t1.arr[i].value);
    }

    table_deinit(&t1);
    return 0;
}
