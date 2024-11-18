#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "hashtable.h"

#define MAX_LEN 100

// Function to trim newline characters from a string
void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
}

int main(int argc, char* argv[]) {

    struct hash_table *table = create_hash_table(200);

    int fd[2]; // fd[0] => read, fd[1] => write
    if (pipe(fd) == -1) return 1;

    int pid = fork(); // Duplicate the process
    if (pid == -1) return 2; // Check for errors

    if(pid == 0){ // child process
        // will write all the words
        close(fd[0]); // we don't read here

        FILE *file = fopen("test.txt", "r");
        if (!file) {
            printf("error opening file");
            close(fd[1]);
            return 3;
        }

        // char str[200] = "I am really good at programing!";        
        // str[strlen(str) - 1] = '\0';

        char line[MAX_LEN];
        printf("Test");
        while(fgets(line, MAX_LEN, file)){
            trim_newline(line);

            int n = strlen(line) + 1;
            if(write(fd[1], &n, sizeof(int)) < 0){
                fclose(file);
                close(fd[1]);
                return 4;
            }
            if (write(fd[1], line, sizeof(char) * n) < 0){
                fclose(file);
                close(fd[1]);
                return 5;
            }
        }
        fclose(file);
        close(fd[1]);
    } else { // parent process 
        // will read and process the words
        close(fd[1]); // we don't write here  
        char str[MAX_LEN];
        int n;

        while(read(fd[0], &n, sizeof(int)) > 0){
            if(read(fd[0], str, sizeof(char) * n) > 0){
                insert_hash_table(table, str);
                // printf("Received: %s\n", str);
            }
        }

        close(fd[0]);
        wait(NULL);
        print_hash_table(table);
    }
    
    return 0;
}
