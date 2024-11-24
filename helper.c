#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

ssize_t read_nbytes(int fd, void *buffer, size_t n) {
    size_t total_read = 0;
    char *buf = buffer;
    while (total_read < n) {
        ssize_t bytes_read = read(fd, buf + total_read, n - total_read);
        if (bytes_read == 0) {
            // EOF detected
            if (total_read == 0) {
                return -1; // Indicate EOF
            } else {
                break; // Exit the loop
            }
        } else if (bytes_read < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by signal, retry
            }
            return -1; // Error
        }
        total_read += bytes_read;
    }
    return total_read;
}



ssize_t write_nbytes(int fd, const void *buffer, size_t n) {
    size_t total_written = 0;
    const char *buf = buffer;
    while (total_written < n) {
        ssize_t bytes_written = write(fd, buf + total_written, n - total_written);
        if (bytes_written <= 0) {
            if (bytes_written < 0 && errno == EINTR) {
                continue; // Interrupted by signal, retry
            }
            return -1; // Error
        }
        total_written += bytes_written;
    }
    return total_written;
}
