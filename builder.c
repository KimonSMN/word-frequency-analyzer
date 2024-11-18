#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


void builder(int builderIndex, int numOfSplitters, int numOfBuilders, int pipes[numOfSplitters][numOfBuilders][2]) {

    for (int s = 0; s < numOfSplitters; s++) {
        close(pipes[s][numOfBuilders][1]); // Close write ends
    }

    printf("Builder %d is reading from pipes...\n", builderIndex);

    // Read from all splitters
    for (int s = 0; s < numOfSplitters; s++) {
        while (1) {

            int n; 
            char str[200];
            if(read(pipes[s][builderIndex][0], &n, sizeof(int)) <= 0){
                break;
            }
            if(read(pipes[s][builderIndex][0], str, sizeof(char) * n) <= 0){
                break;
            }
        
            printf("Builder %d received: %s\n", builderIndex, str);
        

        }
        // Close the read end for the current splitter
        close(pipes[s][builderIndex][0]);
    }

    printf("Builder %d finished reading.\n", builderIndex);
}