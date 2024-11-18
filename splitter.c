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


void splitter(struct hash_table *table, int splitterIndex, int numOfSplitters, int numOfBuilders, char *inputFile, int inputFileLines ,int pipes[][2]){

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

    // Process lines in the range [sectionFrom, sectionTo)
    char* token;
    char* delim = " "; 
    
    for(int i = sectionFrom; i < sectionTo; i++){
        // logic

        getline(&line, &len, file);
        clean_text(line);
        token = strtok(line, delim);

        while (token) {
            insert_hash_table(table, token);
            token = strtok(NULL, delim);
        }
        // printf("%s\n", line);

    }

    fclose(file);
}


int main() {
    char *inputFile = "numbers.txt"; // Example input file
    int numOfSplitters = 4;       // Number of splitters
    int numOfBuilders = 3;        // Example builders count (not used in this test)

    struct hash_table *table = create_hash_table(100);

    // Count the total lines in the file
    int inputFileLines = 17;
    if (inputFileLines == -1) {
        perror("Failed to count lines");
        exit(1);
    }
    printf("Total lines in the file: %d\n", inputFileLines);

    // Simulate pipes for testing
    int pipes[numOfBuilders][2];
    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(pipes[b]) == -1) {
            perror("Pipe creation failed");
            return 1;
        }
    }

    // Test each splitter
    for (int i = 0; i < numOfSplitters; i++) {
        printf("\n--- Splitter %d Output ---\n", i);
        splitter(table, i, numOfSplitters, numOfBuilders, inputFile, inputFileLines, pipes);
    }

    print_hash_table(table);
    return 0;
}