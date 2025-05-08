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
#include "helper.h"

int count_lines(const char* filename) {
    
    FILE *fp = fopen(filename, "r"); 
    if (!fp) {
        perror("Error: Opening file for reading failed");
        return -1;
    }

    int numOfLines = 0;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0){
        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') 
                numOfLines++;
        }
    }

    fclose(fp);
    return numOfLines + 1;
}


ssize_t safe_read(int fd, void *buffer, size_t n) {
    size_t readSum = 0;
    char *charbuffer = (char *)buffer;

    while (readSum < n) {
        charbuffer += readSum;
        n -= readSum;
        ssize_t bytesRead = read(fd, charbuffer, n);
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Error: Safe read failed.");
            exit(1);
        }
        if (bytesRead == 0){
            break;
        } 
        readSum += bytesRead;
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

char *trim_space(char *str){
    
    while(isspace((unsigned char)*str)){
        str++;
    }

    if(*str == '\0'){
        return str;
    }

    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)){
        end--;
    }

    end[1] = '\0';
    return str;
}

// Compare Function for qsort().
int compare_frequency(const void *a, const void *b) {
    struct hash_node *nodeA = *(struct hash_node **)a;
    struct hash_node *nodeB = *(struct hash_node **)b;
    return nodeB->count - nodeA->count; // Decending order.
}
