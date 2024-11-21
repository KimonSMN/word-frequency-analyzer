#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

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

bool isExcluded(char *word, char *exclusionList[], int exclusionListSize){
    for (int i = 0; i < exclusionListSize; i++) {
        if (strcmp(word, exclusionList[i]) == 0) {
            return true;
        }
    }
    return false;
}



void splitter(int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines, int builderPipes[numOfBuilders][2], int exclusionListSize ,char *exclusionList[]){


    // Initialize buffers for each builder
    char **builderBuffers = malloc(numOfBuilders * sizeof(char *));
    size_t *builderBufferSizes = malloc(numOfBuilders * sizeof(size_t));

    for (int b = 0; b < numOfBuilders; b++) {
        builderBuffers[b] = NULL;
        builderBufferSizes[b] = 0;
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
    char *delim = " \t\n";
    
    for (int i = sectionFrom; i < sectionTo; i++) {
        if (getline(&line, &len, file) == -1) {
            perror("Error reading line");
            fclose(file);
            exit(1);
        }

        clean_text(line);
        trim_newline(line);
        token = strtok(line, delim);

        while (token) {
            if (isExcluded(token, exclusionList, exclusionListSize)) { // If token excluded skip to the next one
                token = strtok(NULL, delim);
                continue;   
            }
                    
            // Hash the word to get the builder index
            unsigned long bucketForWord = hash(token, 100);
            int builderIndex = bucketForWord % numOfBuilders;

            
            // Calculate new size
            size_t oldSize = builderBufferSizes[builderIndex];
            size_t tokenLen = strlen(token);
            size_t newSize;

            if (builderBufferSizes[builderIndex] == 0) {
                // First word
                newSize = oldSize + tokenLen + 1; // +1 for null terminator
            } else {
                newSize = oldSize + 1 + tokenLen + 1; // +1 for space, +1 for null terminator
            }

            char *temp = realloc(builderBuffers[builderIndex], newSize);
            if (temp == NULL) {
                perror("Realloc failed");
                exit(1);
            }
            builderBuffers[builderIndex] = temp;

            // Append the word and a space (or newline)
            if (builderBufferSizes[builderIndex] == 0) {
                // First word, no need for space
                strcpy(builderBuffers[builderIndex], token);
                builderBufferSizes[builderIndex] = tokenLen;

            } else {
                strcat(builderBuffers[builderIndex], " ");
                strcat(builderBuffers[builderIndex], token);
                builderBufferSizes[builderIndex] = oldSize + 1 + tokenLen;
            }

            printf("Splitter %d queues '%s' for Builder %d\n", splitterIndex, token, builderIndex);

            token = strtok(NULL, delim);
        }
    }

    // Send merged words to each builder
    for (int b = 0; b < numOfBuilders; b++) {
        if (builderBuffers[b] != NULL && builderBufferSizes[b] > 0) {
            // Send the size of the buffer first
            int n = builderBufferSizes[b] + 1; // Include null terminator
            if (write(builderPipes[b][1], &n, sizeof(int)) < 0) {
                perror("Error writing size to pipe");
            }

            // Send the buffer
            if (write(builderPipes[b][1], builderBuffers[b], n) < 0) {
                perror("Error writing buffer to pipe");
            }

            printf("Splitter %d sends merged words to Builder %d\n", splitterIndex, b);
        }
    }

    // Free allocated memory
    for (int b = 0; b < numOfBuilders; b++) {
        free(builderBuffers[b]);
    }
    free(builderBuffers);
    free(builderBufferSizes);

    free(line);
    fclose(file);
}
