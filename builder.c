#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

ssize_t safe_read(int fd, void *buf, size_t count) {
    size_t totalBytesRead = 0;
    char *buffer = (char *)buf;

    while (totalBytesRead < count) {
        ssize_t bytesRead = read(fd, buffer + totalBytesRead, count - totalBytesRead);
        if (bytesRead > 0) {
            totalBytesRead += bytesRead;
        } else if (bytesRead == 0) {
            // EOF reached
            break;
        } else if (errno == EINTR) {
            // Interrupted by signal, retry read
            continue;
        } else {
            // Other error occurred
            return -1;
        }
    }

    return totalBytesRead;
}

void builder(int builderIndex, int numOfSplitters, int numOfBuilders, int splitterToBuilder[numOfSplitters][numOfBuilders][2]) {

    for (int s = 0; s < numOfSplitters; s++) {
        close(splitterToBuilder[s][builderIndex][1]); // Close write ends
    }

    printf("Builder %d is reading from pipes...\n", builderIndex);

    for (int s = 0; s < numOfSplitters; s++) {
        while(1){
            int n; 
            char str[200];
            size_t bytes;

            read(splitterToBuilder[s][builderIndex][0], &n, sizeof(int));
 
            // printf("Length %d ", n);
            bytes = read(splitterToBuilder[s][builderIndex][0], str, sizeof(char) * n);
            if(bytes < 0){
                exit(1);
            } else if (bytes == 0){
                // EOF
                printf("Reached EOF\n");
                break;
            }

            // printf("Word %s\n", str);

            printf("Builder %d received: %s From Splitter: %d\n ", builderIndex, str, s);
        
        }

        close(splitterToBuilder[s][builderIndex][0]);   // Close the read end

    }

    printf("Builder %d finished reading.\n", builderIndex);
}
