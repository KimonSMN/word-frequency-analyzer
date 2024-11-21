#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

#include <arpa/inet.h> // For htonl and ntohl

#include "splitter.h"
#include "builder.h"

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
    return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

int main(int argc, char *argv[]) {

    //////// HANDLE COMMAND LINE ARGUMENTS ////////

    char *inputFileName = NULL;
    int numOfSplitters = 0;
    int numOfBuilders = 0;
    int topK = 0;
    char *exclusionFileName = NULL;
    char *outputFile = NULL;

    // Parse the arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            inputFileName = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            numOfSplitters = atoi(argv[++i]);        // Number of Splitters initialized in the command line
        } else if (strcmp(argv[i], "-m") == 0) {
            numOfBuilders = atoi(argv[++i]);        // Number of Builders initialized in the command line
        } else if (strcmp(argv[i], "-t") == 0) {
            topK = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0) {
            exclusionFileName = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFile = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    //////// READ FILE LINES ////////
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(1);
    }

    int current_line = 0;
    int ch;  // Use int to accommodate EOF value
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch == '\n') {
            current_line++;
        }
    }
    // If the last line doesn't end with a newline character
    // fseek(inputFile, 0, SEEK_END);
    // if (ftell(inputFile) > 0) {
    //     fseek(inputFile, -1, SEEK_END);
    //     if (fgetc(inputFile) != '\n') {
    //         current_line++;
    //     }
    // }
    printf("FILE HAS %d LINES\n", current_line);
    fclose(inputFile);

    //////// READ SIZE OF EXCLUSION FILE ////////
    FILE *exclusionFile = fopen(exclusionFileName, "r");

    int exclusionFileLines = 0;
    // Reset ch for reuse
    while ((ch = fgetc(exclusionFile)) != EOF) {
        if (ch == '\n') {
            exclusionFileLines++;
        }
    }
    // // Handle last line not ending with a newline
    // fseek(exclusionFile, 0, SEEK_END);
    // if (ftell(exclusionFile) > 0) {
    //     fseek(exclusionFile, -1, SEEK_END);
    //     if (fgetc(exclusionFile) != '\n') {
    //         exclusionFileLines++;
    //     }
    // }
    printf("EXCLUSION HAS %d LINES\n", exclusionFileLines);

    // Allocate memory for the exclusion list
    char line[128];
    char **exclusionList = malloc(exclusionFileLines * sizeof(char *));
    int i = 0;

    rewind(exclusionFile);

    while (fgets(line, sizeof(line), exclusionFile)) {
        line[strcspn(line, "\r\n")] = '\0';
        trimwhitespace(line);
        exclusionList[i] = strdup(line);
        i++;
    }
    fclose(exclusionFile);

    //////// CREATE PIPES ////////
    // pipe[0] => read, 
    // pipe[1] => write

    
    int builderPipes[numOfBuilders][2]; // [][0] for reading by builders, [][1] for writing by splitters

    // Create one pipe per builder
    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(builderPipes[b]) == -1) {
            perror("Pipe creation failed");
            exit(1);
        }
    }

    //////// FORK SPLITTERS ////////

    for (int s = 0; s < numOfSplitters; s++) {
        int pid = fork();
        if (pid == -1) {
            perror("Error creating splitter process");
            return 2;
        }

        if (pid == 0) { // Child process (splitter)
            for(int b = 0;b < numOfBuilders; b++){
                close(builderPipes[b][0]);  // Close read ends of each builder pipe
            }

            // ADD LOGIC HERE

            splitter(s, numOfSplitters, numOfBuilders, inputFileName, current_line, builderPipes, exclusionFileLines, exclusionList);

            // Close write ends before exiting
            for (int b = 0; b < numOfBuilders; b++) {
                close(builderPipes[b][1]); // Close write ends
            }

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

        if (pid == 0) { // Child process

            // ADD LOGIC HERE

            for (int i = 0; i < numOfBuilders; i++) {
                close(builderPipes[i][1]); // Close write ends
                if (i != b) {
                    close(builderPipes[i][0]); // Close read ends of other builders
                }
            }

            
            builder(b, numOfSplitters, numOfBuilders, builderPipes);


            // Close read end before exiting
            close(builderPipes[b][0]);

            // Exit after processing
            return 1;
        }

    }


    // PARENT PROCESS

    // Cloes all pipe ends
    for (int b = 0; b < numOfBuilders; b++) {
        close(builderPipes[b][0]);
        close(builderPipes[b][1]);
    }
    

    // Wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }



    return 0;
}