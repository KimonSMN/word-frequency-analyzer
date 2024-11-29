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
#include "helper.h"

bool isExcluded(char *word, char *exclusionList[], int exclusionListSize){
    for (int i = 0; i < exclusionListSize; i++) {
        if (strcmp(word, exclusionList[i]) == 0) {
            return true;
        }
    }
    return false;
}

void splitter(int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines, int builderPipes[numOfBuilders][2], int exclusionListSize ,char *exclusionList[]){

    // Initialize buffers.
    // We use calloc to avoid manually setting the values to NULL & 0.
    char **builderBuffers = calloc(numOfBuilders, sizeof(char*));
    if (builderBuffers == NULL){    // Error checking.
        perror("Memory allocation failed for builderBuffers");
        exit(1);
    }

    size_t *builderBufferSizes = calloc(numOfBuilders, sizeof(size_t));
    if (builderBufferSizes == NULL){    // Error checking.
        perror("Memory allocation failed for builderBufferSizes");
        exit(1);
    }

    FILE *file = fopen(inputFile, "r"); // Open inputFile.
    if (file == NULL) {
        perror("Error opening input file");
        exit(1);
    }
    int sectionFrom = splitterIndex * (inputFileLines / numOfSplitters); // Starting section (Splitter starts reading from here).
    int sectionTo = sectionFrom + (inputFileLines / numOfSplitters);     // Ending section  (Splitter ends the reading here).
    if (sectionTo > inputFileLines) {
        sectionTo = inputFileLines; // Ensure the last splitter processes up to the last line.
    }

    // If splitterIndex is the last splitter,
    // it gets all the remaining file lines.
    if (splitterIndex == numOfSplitters - 1) {  
        sectionTo = inputFileLines;
    }

    // Skip lines until sectionFrom.
    size_t len = 0;
    char *line = NULL;
    for (int currentLine = 0; currentLine < sectionFrom; currentLine++) {
        if (getline(&line, &len, file) == -1) {
            perror("Error: Skipping lines failed");
            free(line);
            fclose(file);
            exit(1);
        }
    }

    // Process lines from sectionFrom to sectionTo
    char* token;
    char *delim = " \t\n";
    
    for (int i = sectionFrom; i < sectionTo; i++) {
        if (getline(&line, &len, file) == -1) {
            perror("Error: Reading line failed");
            free(line);
            fclose(file);
            exit(1);
        }

        clean_string(line);
        trim_newline(line);
        
        token = strtok(line, delim);

        while (token) {
            if (isExcluded(token, exclusionList, exclusionListSize)) { // If token excluded, skip to the next one.
                token = strtok(NULL, delim);
                continue;   
            }
                    
            // Hash the word to get the builder index.
            unsigned long bucketForWord = hash((unsigned char *)token);
            int builderIndex = bucketForWord % numOfBuilders;

            // Calculate new size.
            size_t oldSize = builderBufferSizes[builderIndex];
            size_t tokenLen = strlen(token);
            size_t newSize;

            if (builderBufferSizes[builderIndex] == 0) {
                newSize = oldSize + tokenLen + 1; // First word, add +1 for null terminator.
            } else {
                newSize = oldSize + 1 + tokenLen + 1; // Not first word, add +1 for space and +1 for null terminator.
            }

            char *temp = realloc(builderBuffers[builderIndex], newSize);
            if (temp == NULL) { // Error checking. 
                perror("Error: Realloc failed");
                for (int b = 0; b < numOfBuilders; b++) {
                    free(builderBuffers[b]);
                }
                free(builderBuffers);
                free(builderBufferSizes);
                free(line);
                fclose(file);
                exit(1);
            }
            builderBuffers[builderIndex] = temp;

            if (builderBufferSizes[builderIndex] == 0) {
                // First word, no space needed.
                strcpy(builderBuffers[builderIndex], token);
                builderBufferSizes[builderIndex] = tokenLen;
            } else {
                strcat(builderBuffers[builderIndex], " ");
                strcat(builderBuffers[builderIndex], token);
                builderBufferSizes[builderIndex] = oldSize + 1 + tokenLen;
            }
            token = strtok(NULL, delim);
        }
    }

    // Send merged words to each builder.
    for (int b = 0; b < numOfBuilders; b++) {
        if (builderBuffers[b] != NULL && builderBufferSizes[b] > 0) {
            int n = builderBufferSizes[b] + 1;
            if (write(builderPipes[b][1], &n, sizeof(int)) < 0) { // Send buffer size.
                perror("Error writing size to pipe");
            }

            if (write(builderPipes[b][1], builderBuffers[b], n) < 0) { // Send the buffer.
                perror("Error writing buffer to pipe");
            }
        }
        close(builderPipes[b][1]);  // Close the write end of the pipe
    }

    // Free allocated memory
    for (int b = 0; b < numOfBuilders; b++) {
        free(builderBuffers[b]);
    }
    free(builderBuffers);
    free(builderBufferSizes);
    free(line);
    fclose(file);
    kill(getppid(), SIGUSR1);
}
