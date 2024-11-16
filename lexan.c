#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    int fd[2]; // fd[0] => read, fd[1] => write
    if (pipe(fd) == -1) return 2;

    int pid = fork(); // Duplicate the process
    if (pid == -1) return 3; // Check for errors

    if(pid == 0){
        close(fd[0]); // we don't read here
        // child process
        // will write all the words
        int file = open("test.txt", O_RDONLY); // open the file

        char buffer[2056];
        ssize_t bytesRead;

        while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
            if(write(fd[1], buffer, bytesRead) == -1) {
                close(file);
                close(fd[1]);
                return 5;
            }
        }
        close(file);
        close(fd[1]);
    } else{
        close(fd[1]); // we don't write here 
        // parent process
        // will read and process the words
        char buffer[2056];
        ssize_t bytesRead;

        while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, bytesRead);
        }

        close(fd[0]);
    }

    return 0;
}
