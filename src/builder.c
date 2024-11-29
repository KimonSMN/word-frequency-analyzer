#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>


#include "hashtable.h"
#include "helper.h"

void builder(int builderIndex, int numOfBuilders, int builderPipes[numOfBuilders][2], int builderToRootPipes[numOfBuilders][2], int inputFileLines, int builderTimingPipes[numOfBuilders][2]) {

    clock_t startClock = clock();

    // Initialize Hash Table. 
    int wordsPerBuilder = (inputFileLines * 10) / numOfBuilders;    // Approximately 10 words per line.
    int uniqueWords = wordsPerBuilder * 0.5;                        // 50% of the words are unique.
    int hashTableCapacity = get_hash_table_capacity(uniqueWords);   // Find a "good" size for the hash table based on the uniqueWords.
    struct hash_table *table = create_hash_table(hashTableCapacity);// Create the hash table.

    // Initialize variables.
    char *buffer = NULL;

    while (1) { // It loops until EOF, or until something goes wrong.
        int n;
        ssize_t result = safe_read(builderPipes[builderIndex][0], &n, sizeof(int));  // Read the size n of the incoming data
        if (result == 0) {
            break;
        } else if (result < 0) {
            perror("Error reading size from pipe");
            exit(1);
        } else if (result != sizeof(int)) {
            fprintf(stderr, "Partial read of size\n");
            exit(1);
        }

        if(n == 0){
            break;
        }

        // Allocate buffer to receive the data
        buffer = realloc(buffer, n); 
        if (buffer == NULL) {
            perror("Realloc failed");
            exit(1);
        }

        // Read the buffer
        result = safe_read(builderPipes[builderIndex][0], buffer, n);
        if (result != n) {
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
    send_hash_table_to_root(table, builderToRootPipes[builderIndex][1]);

    clock_t endClock = clock();

    double timeTaken = (double)(endClock - startClock) / CLOCKS_PER_SEC;

    // Send timing information to the root process.
    if (write(builderTimingPipes[builderIndex][1], &timeTaken, sizeof(double)) < 0) {
        perror("Error sending timing information");
        exit(1);
    }
    close(builderTimingPipes[builderIndex][1]); // Close the write end of the timing pipe.
    close(builderPipes[builderIndex][0]);       // Close the read end after processing input.
    close(builderToRootPipes[builderIndex][1]); // Close the write end after sending data to the root process.

    // Cleanup
    destroy_hash_table(table); 
    free(buffer);
    kill(getppid(), SIGUSR2);
}


