#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

int main(int argc, char *argv[]) {
    struct hash_table *table = create_hash_table(100);

    char str[80] = "This is www.tutorialspoint.com website test test test";
    char *token;

    /* Get the first token */
    token = strtok(str, " ");

    /* Walk through other tokens */
    while (token != NULL) {
        insert_hash_table(table, token);
        token = strtok(NULL," ");
    }

    print_hash_table(table);
    destroy_hash_table(table); // Always clean up memory
    return 0;
}