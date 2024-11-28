#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

ssize_t safe_read(int fd, char *buffer, size_t n) {
    size_t total_read = 0;

    while (total_read < n) {
        ssize_t bytes_read = read(fd, buffer + total_read, n - total_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue; // Retry on signal interruption
            perror("safe_read");
            return -1;
        }
        if (bytes_read == 0) break; // EOF
        total_read += bytes_read;
    }
    return total_read;
}

