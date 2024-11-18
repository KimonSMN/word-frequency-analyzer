#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "hashtable.h"

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
}

void clean_text(char *str) {
    
    size_t len = strlen(str);
    int j = 0; // Index for the updated string

    for (int i = 0; i < len; i++) {
        if((str[i] >= 65 && str[i] <= 90 || str[i] >= 97 && str[i] <= 122 || str[i] == ' ')) {
            str[j++] = str[i]; // Copy it to the new position
        }
    }
    str[j] = '\0';
}


void splitter(int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines, int pipes[numOfSplitters][numOfBuilders][2]){

    for (int b = 0; b < numOfBuilders; b++) {
        close(pipes[splitterIndex][b][0]); // Close read ends
    }

    FILE *file = fopen(inputFile, "r");

    int sectionFrom = splitterIndex * (inputFileLines / numOfSplitters); // splitter_2 * (1000 lines / 4) => 2 * 250 => 500 
    int sectionTo = sectionFrom + (inputFileLines / numOfSplitters);     // 500 + (1000/4) => 750
    if (splitterIndex == numOfSplitters - 1) {
        sectionTo = inputFileLines; // Ensure the last splitter gets all remaining lines
    }

    // Skip lines until sectionFrom
    size_t len = 0;
    char *line = NULL;
    for (int currentLine = 0; currentLine < sectionFrom; currentLine++) {
        if (getline(&line, &len, file) == -1) {
            perror("Error skipping lines");
            fclose(file);
            exit(1);
        }
    }

    // Process lines from sectionFrom to sectionTo
    char* token;
    char* delim = " "; 
    
    for(int i = sectionFrom; i < sectionTo; i++){
        // logic
        getline(&line, &len, file);
        clean_text(line);
        trim_newline(line);
        token = strtok(line, delim);

        while (token) {
            // insert_hash_table(table, token);
            // printf("Token: %s\n", token);
            unsigned long bucketForWord = hash(token, 100);
            int builderIndex = bucketForWord % numOfBuilders;
            // printf("Splitter %d sends '%s' to Builder %d\n", splitterIndex, token, builderIndex);

            // Send Word to builder
            int n = strlen(token) + 1;

            printf("Splitter %d writes length %d for '%s' to Builder %d\n", splitterIndex, n, token, builderIndex);
            if (write(pipes[splitterIndex][builderIndex][1], &n, sizeof(int)) < 0) {
                perror("Error writing n to pipe");
            }

            printf("Splitter %d writes '%s' to Builder %d\n", splitterIndex, token, builderIndex);
            if (write(pipes[splitterIndex][builderIndex][1], token, sizeof(char) * n) < 0) {
                perror("Error writing to pipe");
            }
            token = strtok(NULL, delim);
        }
    }

    for (int b = 0; b < numOfBuilders; b++) {
        printf("Splitter %d finished sending and closed pipes\n", splitterIndex);
        close(pipes[splitterIndex][b][1]); // Close write ends
    }
    fclose(file);
}
