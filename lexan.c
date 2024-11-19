#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


#include "splitter.h"
#include "builder.h"

int main(int argc, char *argv[]) {

    //////// HANDLE COMMAND LINE ARGUMENTS ////////

    char *inputFile = NULL;
    int numOfSplitters = 0;
    int numOfBuilders = 0;
    int topK = 0;
    char *exclusionList = NULL;
    char *outputFile = NULL;

    // Parse the arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            numOfSplitters = atoi(argv[++i]);        // Number of Splitters initialized in the command line
        } else if (strcmp(argv[i], "-m") == 0) {
            numOfBuilders = atoi(argv[++i]);        // Number of Builders initialized in the command line
        } else if (strcmp(argv[i], "-t") == 0) {
            topK = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0) {
            exclusionList = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFile = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    //////// CREATE PIPES ////////
    int splitterToBuilder[numOfSplitters][numOfBuilders][2];    // pipe[0] => read, 
    int builderToParent[numOfSplitters][2];                      // pipe[1] => write

    for (int s = 0; s < numOfSplitters; s++) {  // Splitters
        for (int b = 0; b < numOfBuilders; b++) {   // Builders
            if (pipe(splitterToBuilder[s][b]) == -1) {
                printf("Splitter pipe creation failed\n");
                return 1;
            }
        }
    }

    for (int b = 0; b < numOfBuilders; b++) {   // Builders
        if (pipe(builderToParent[numOfBuilders]) == -1 ){
            printf("Builder pipe creation failed\n");
            return 1;
        }
    }

    //////// FORK SPLITTERS ////////

    int builderIndex;
    for (int s = 0; s < numOfSplitters; s++) {
        int pid = fork();
        if (pid == -1) {
            perror("Error creating splitter process");
            return 2;
        }

        if (pid == 0) { // Child process (splitter)
            // Close unused pipes for this splitter
            for (int otherSplitter = 0; otherSplitter < numOfSplitters; otherSplitter++) {
                for (int b = 0; b < numOfBuilders; b++) {
                    if (otherSplitter != s) {
                        close(splitterToBuilder[otherSplitter][b][0]); // Close read ends
                        close(splitterToBuilder[otherSplitter][b][1]); // Close write ends
                    }
                }
            }

            for (int b = 0; b < numOfBuilders; b++) {
                close(splitterToBuilder[s][b][0]); // Close read ends
            }

            FILE *file = fopen(inputFile, "r");

            int sectionFrom = s * (8 / numOfSplitters); // splitter_2 * (1000 lines / 4) => 2 * 250 => 500 
            int sectionTo = sectionFrom + (8 / numOfSplitters);     // 500 + (1000/4) => 750
            if (s == numOfSplitters - 1) {
                sectionTo = 8; // Ensure the last splitter gets all remaining lines
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
                    // printf("Splitter %d sends '%s' to Builder %d\n", s, token, builderIndex);

                    // Send Word to builder
                    int n = strlen(token) + 1;
                    if (write(splitterToBuilder[s][builderIndex][1], &n, sizeof(int)) < 0) {
                        perror("Error writing n to pipe");
                    }

                    printf("Splitter %d writes '%s' to Builder %d\n", s, token, builderIndex);
                    if (write(splitterToBuilder[s][builderIndex][1], token, sizeof(char) * n) < 0) {
                        perror("Error writing to pipe");
                    }
                    token = strtok(NULL, delim);
                }
            }

            for (int b = 0; b < numOfBuilders; b++) {
                close(splitterToBuilder[s][b][1]); // Close write ends
            }
            printf("Splitter %d finished sending and closed pipes\n", s);

            free(line);
            fclose(file);

            // Exit after processing
            return 1;
        }
    }


    //////// FORK BUILDERS ////////
    for (int b = 0; b < numOfBuilders; b++) {
        int pid = fork();
        if (pid == -1) {
            perror("Error creating builder process");
            return 2;
        }

        if (pid == 0) { // Child process (builder)
            
            for (int s = 0; s < numOfSplitters; s++) {
                    close(splitterToBuilder[s][builderIndex][1]); // Close write ends
                }

            printf("Builder %d is reading from pipes...\n", builderIndex);

            for (int s = 0; s < numOfSplitters; s++) {
                while(1){
                    int n; 
                    char str[200];
                    size_t bytes;

                    read(splitterToBuilder[s][builderIndex][0], &n, sizeof(int));
        
                    // printf("Length %d ", n);
                    bytes = read(splitterToBuilder[s][builderIndex][0], str, sizeof(char) * n);
                    if(bytes == -1){
                        return 1;
                    } else if (bytes == 0){
                        // EOF
                        printf("Reached EOF\n");
                        break;
                    }

                    // printf("Word %s\n", str);

                    printf("Builder %d received: %s From Splitter: %d\n ", builderIndex, str, s);
                
                }

                close(splitterToBuilder[s][builderIndex][0]);   // Close the read end

            }

            printf("Builder %d finished reading.\n", builderIndex);
            // Exit after processing
            return 1;
        }
    }


    // PARENT PROCESS



    // Cloes all pipe ends
    for (int s = 0; s < numOfSplitters; s++) {
        for (int b = 0; b < numOfBuilders; b++) {
            close(splitterToBuilder[s][b][0]);
            close(splitterToBuilder[s][b][1]);
        }
    }

    // Wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }


    fclose(inputFile);

    return 0;
}