#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include "hashtable.h"

void builder(int builderIndex, int numOfSplitters, int numOfBuilders, int builderPipes[numOfBuilders][2], int builderToRootPipes[builderIndex][2]) {
    // Read the size of the incoming data
    // Initialize variables

    // Create hash table

    struct hash_table *table = create_hash_table(3079); // maybe pass the lines of the file and split by the builders and get the size as  the capacity
    
    int writeFd = builderToRootPipes[builderIndex][1];

    char *buffer = NULL;
    size_t bufferSize = 0;

    while (1) {
        // Read the size of the incoming data
        int n;
        ssize_t nbytes = read(builderPipes[builderIndex][0], &n, sizeof(int));

        if (nbytes == 0) {
            // EOF detected, no more data
            break;
        } else if (nbytes < 0) {
            perror("Error reading size from pipe");
            exit(1);
        } else if (nbytes != sizeof(int)) {
            fprintf(stderr, "Partial read of size\n");
            exit(1);
        }

        // Allocate buffer to receive the data
        buffer = realloc(buffer, n); 
        if (buffer == NULL) {
            perror("Realloc failed");
            exit(1);
        }

        // Read the buffer
        nbytes = read(builderPipes[builderIndex][0], buffer, n);
        if (nbytes != n) {
            perror("Error reading buffer from pipe");
            exit(1);
        }

        // Process the merged words
        char *token;
        char *delim = " \t\n";
        token = strtok(buffer, delim);

        while (token) {
            // Process each word
            printf("Builder %d processes word '%s'\n", builderIndex, token);
            
            // insert to hash table
            insert_hash_table(table, token);

            token = strtok(NULL, delim);
        }
    }

    send_hash_table_to_root(table, writeFd);
    // send hash table to root
    // free hash table
    print_hash_table(table);
    // Free allocated memory
    free(buffer);
}


