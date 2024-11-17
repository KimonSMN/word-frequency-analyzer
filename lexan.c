#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "hashtable.h"

int main(int argc, char* argv[]) {

    struct hash_table *table = create_hash_table(200);

    int fd[2]; // fd[0] => read, fd[1] => write
    if (pipe(fd) == -1) return 1;

    int pid = fork(); // Duplicate the process
    if (pid == -1) return 2; // Check for errors

    if(pid == 0){ // child process
        // will write all the words
        close(fd[0]); // we don't read here

        char str[200] = "I am really good at programing!";        
        str[strlen(str) - 1] = '\0';

        int n = strlen(str) + 1;
        if(write(fd[1], &n, sizeof(int)) < 0){
            return 3;
        }

        if (write(fd[1], str, sizeof(char) * n) < 0){
            return 4;
        }

        close(fd[1]);
    } else { // parent process 
        // will read and process the words
        close(fd[1]); // we don't write here  
        char str[200];
        int n;

        if(read(fd[0], &n, sizeof(int)) < 0){
            return 5;
        }
        if(read(fd[0], str, sizeof(char) * n) < 0){
            return 6;
        }

        insert_hash_table(table, str);
        printf("Recieved: %s\n", str);
        close(fd[0]);
        wait(NULL);
        print_hash_table(table);
    }
    
    return 0;
}
