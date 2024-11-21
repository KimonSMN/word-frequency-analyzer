#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

ssize_t safe_read(int fd, void *buffer, size_t count) {
    size_t bytes_read = 0;
    ssize_t result;
    while (bytes_read < count) {
        result = read(fd, (char *)buffer + bytes_read, count - bytes_read);
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (result == 0) break;
        bytes_read += result;
    }
    return bytes_read;
}


void builder(int builderIndex, int numOfSplitters, int numOfBuilders, int builderPipes[numOfBuilders][2]) {
    // Read the size of the incoming data
    // Initialize variables
    char *buffer = NULL;
    size_t bufferSize = 0;
    const int MAX_BUFFER_SIZE = 1024 * 1024; // 1 MB, adjust as needed

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
            token = strtok(NULL, delim);
        }
    }

    // Free allocated memory
    free(buffer);
}


