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
#include "helper.h"
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

    int current_line = 1;
    int ch;  // Use int to accommodate EOF value
    while ((ch = fgetc(inputFile)) != EOF) {
        if (ch == '\n') {
            current_line++;
        }
    }

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
 
    // Pipes for Splitters & Builders
    int builderPipes[numOfBuilders][2]; // [][0] for reading by builders, [][1] for writing by splitters

    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(builderPipes[b]) == -1) {
            perror("Pipe creation failed");
            exit(1);
        }
    }

    // Pipes for Builders & Root

    int builderToRootPipes[numOfBuilders][2]; // [][0] for reading by root, [][1] for writing by builders

    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(builderToRootPipes[b]) == -1) {
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

            for (int i = 0; i < numOfBuilders; i++) {
                close(builderPipes[i][1]); // Close write ends
                if (i != b) {
                    close(builderPipes[i][0]); // Close read ends of other builders
                }
                close(builderToRootPipes[i][0]); // Close read ends of builderToRootPipes
                if (i != b) {
                    close(builderToRootPipes[i][1]); // Close write ends of other builders
                }
            }

            // Now builder can use builderPipes[b][0] to read from splitters
            // And builderToRootPipes[b][1] to write to root
            
            builder(b, numOfBuilders, builderPipes, builderToRootPipes);

            // Close read end before exiting
            close(builderPipes[b][0]);
            close(builderToRootPipes[b][1]);

            // Exit after processing
            return 1;
        }

    }

    for (int b = 0; b < numOfBuilders; b++) {
        close(builderToRootPipes[b][1]); // Close write ends in root
    }   

    // Cloes all pipe ends
    for (int b = 0; b < numOfBuilders; b++) {
        close(builderPipes[b][0]);
        close(builderPipes[b][1]);
    }
    

    // wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }

    // PARENT PROCESS

    for (int b = 0; b < numOfBuilders; b++) {
        close(builderToRootPipes[b][1]); // Close write ends in root

        while(1){
            int n;
            char *buffer = malloc(n);
            int freq;

            ssize_t nbytes = read_nbytes(builderToRootPipes[b][0], &n, sizeof(int));
            if(nbytes < 0){
                return 1;
            } else if(nbytes == 0){
                //EOF
                break;
            }
            if (n == 0) {
                // End marker received
                printf("Root: Received end marker from Builder %d\n", b);
                break;
            }
            nbytes = read_nbytes(builderToRootPipes[b][0], buffer, sizeof(char) * n);
            if(nbytes < 0){
                return 1;
            } 
            nbytes = read_nbytes(builderToRootPipes[b][0], &freq, sizeof(int));
            if(nbytes < 0){
                return 1;
            } 
            printf("Received from Builder %d: %s, %d\n", b, buffer, freq);
        }


        close(builderToRootPipes[b][0]);
    }

    return 0;
}