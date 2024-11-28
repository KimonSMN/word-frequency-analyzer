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
#include "helper.h"

void builder(int builderIndex, int numOfBuilders, int builderPipes[numOfBuilders][2], int builderToRootPipes[numOfBuilders][2], int inputFileLines, pid_t root_pid) {

    // Initialize Hash Table. 
    int wordsPerBuilder = (inputFileLines * 10) / numOfBuilders;    // Approximately 10 words per line.
    int uniqueWords = wordsPerBuilder * 0.5;                        // 50% of the words are unique.
    int hashTableCapacity = get_hash_table_capacity(uniqueWords);   // Find a "good" size for the hash table based on the uniqueWords.
    struct hash_table *table = create_hash_table(hashTableCapacity);// Create the hash table.

    // Initialize variables.
    char *buffer = NULL;

    
    while (1) { // It loops until EOF, or until something goes wrong.
        int n;
        ssize_t nbytes = read(builderPipes[builderIndex][0], &n, sizeof(int));  // Read the size n of the incoming data
        if (nbytes == 0) {
            break;
        } else if (nbytes < 0) {
            perror("Error reading size from pipe");
            exit(1);
        } else if (nbytes != sizeof(int)) {
            fprintf(stderr, "Partial read of size\n");
            exit(1);
        }

        if (n == 0) break; // End marker detected


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
            free(buffer);

            exit(1);
        }

        // Process the merged words
        char *token;
        char *delim = " \t\n";
        token = strtok(buffer, delim);

        while (token) {
            insert_hash_table(table, token);

            token = strtok(NULL, delim);
        }
    }
    int writeFd = builderToRootPipes[builderIndex][1]; // write
    send_hash_table_to_root(table, writeFd);

    close(writeFd); // Close the write end after sending data

    // Cleanup
    destroy_hash_table(table); 

    free(buffer);   

    kill(getppid(), SIGUSR2);

}


