#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#include "hashtable.h"
ssize_t safe_read(int fd, void *buffer, size_t n) {
    size_t readSum = 0;

    while (readSum < n) {
        ssize_t bytesRead = read(fd, buffer, n);
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Error: safe read failed.");
            exit(1);
        }
        if (bytesRead == 0){
            break; // EOF
        } 
        readSum += bytesRead;
        buffer += readSum;
        n -= readSum;
    }
    return readSum;
}

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0){
        if((str[len - 1] == '\n' || str[len - 1] == '\r')){
            str[len- 1] = '\0';
        }
    }
}

void clean_string(char *str) {
    size_t len = strlen(str);
    size_t index = 0; 
    for (size_t i = 0; i < len; i++) {
        if(((str[i] >= 'A' && str[i] <= 'Z' )||( str[i] >= 'a' && str[i] <= 'z' )|| str[i] == ' ')) {
            str[index] = str[i];
            index++;
        }
    }
    str[index] = '\0';
}

// https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way?page=1&tab=scoredesc#tab-top
char *trimwhitespace(char *str){
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
    return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// Compare Function for qsort()
int compare_frequency(const void *a, const void *b) {
    struct hash_node *nodeA = *(struct hash_node **)a;
    struct hash_node *nodeB = *(struct hash_node **)b;
    return nodeB->count - nodeA->count; // Decending order.
}
