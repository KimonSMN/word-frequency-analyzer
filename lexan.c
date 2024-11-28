#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <bits/sigaction.h>


#include "splitter.h"
#include "builder.h"
#include "helper.h"

// global
int completed_splitters = 0;
int completed_builders = 0;

// // Signal Handlers
void handle_sigusr1(){
    completed_splitters++;

}
void handle_sigusr2(){
    completed_builders++;
}
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
int compare_frequency(const void *a, const void *b) {
    struct hash_node *nodeA = *(struct hash_node **)a;
    struct hash_node *nodeB = *(struct hash_node **)b;
    // For descending order
    return nodeB->count - nodeA->count;
}
int main(int argc, char *argv[]) {

    //////// HANDLE COMMAND LINE ARGUMENTS ////////

    char *inputFileName = NULL;
    int numOfSplitters = 0;
    int numOfBuilders = 0;
    int topK = 0;
    char *exclusionFileName = NULL;
    char *outputFileName = NULL;

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
            outputFileName = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }


    struct sigaction sa1;
    sa1.sa_handler = &handle_sigusr1;
    // sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;

    sigaction(SIGUSR1, &sa1, NULL);

    struct sigaction sa2;
    sa2.sa_handler = &handle_sigusr2;
    // sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;

    sigaction(SIGUSR2, &sa2, NULL);


    //////// READ FILE LINES ////////
    FILE *inputFile = fopen(inputFileName, "r");    // Open the inputFile
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(1);
    }

  int inputFileLines = 0; // Initialize to 0
    char *linee = NULL;
    size_t len = 0;

    while (getline(&linee, &len, inputFile) != -1) {
        // Trim the line to remove leading and trailing whitespace
        trimwhitespace(linee);

        // Count the line only if it's not empty
        if (linee[0] != '\0') {
            inputFileLines++;
        }
    }
    printf("Input File has: %d lines\n", inputFileLines);
    fclose(inputFile);  // Close the inputFile 

    //// READ SIZE OF EXCLUSION FILE ////
    FILE *exclusionFile = fopen(exclusionFileName, "r");

    int exclusionFileLines = 0;
    int lastChar = 0;
    int ch;
    // Count lines
    while ((ch = fgetc(exclusionFile)) != EOF) {
        if (ch == '\n') {
            exclusionFileLines++;
        }
        lastChar = ch;
    }

    // If the last character is not a newline, count the final line
    if (lastChar != '\n') {
        exclusionFileLines++;
    }

    // printf("Exclusion has %d lines\n", exclusionFileLines);

    // Allocate memory for the exclusion list
    char line[64];
    char **exclusionList = NULL;
    if (exclusionFileLines > 0) {
        exclusionList = calloc(exclusionFileLines, sizeof(char *));
        if (exclusionList == NULL) {
            perror("Memory allocation failed for exclusionList");
            exit(1);
        }
    }

    int i = 0;
    rewind(exclusionFile);
    while (i < exclusionFileLines && fgets(line, sizeof(line), exclusionFile)) {
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline
        trimwhitespace(line);

        if (line[0] == '\0') { // Skip empty lines
            exclusionFileLines--; // Adjust expected count for skipped lines
            continue;
        }

        exclusionList[i] = strdup(line);
        if (exclusionList[i] == NULL) {
            perror("Memory allocation failed for exclusion list item");
            exit(1);
        }
        i++;
    }

    if (i != exclusionFileLines) {
        printf("Warning: Expected %d exclusion lines, but only processed %d.\n", exclusionFileLines, i);
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

    // pipes for builder's time 

    int builderTimingPipes[numOfBuilders][2]; // [][0] for reading by root, [][1] for writing by builders

    // Create pipes for timing information
    for (int b = 0; b < numOfBuilders; b++) {
        if (pipe(builderTimingPipes[b]) == -1) {
            perror("Pipe creation failed for timing");
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

            splitter(s, numOfSplitters, numOfBuilders, inputFileName, inputFileLines, builderPipes, exclusionFileLines, exclusionList);

            // Close write ends
            for (int b = 0; b < numOfBuilders; b++) {
                close(builderPipes[b][1]);
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
            builder(b, numOfBuilders, builderPipes, builderToRootPipes, inputFileLines, builderTimingPipes);

            close(builderPipes[b][0]);
            close(builderToRootPipes[b][1]);

            // Exit after processing
            return 1;
        }

    }

    // Cloes all pipe ends
    for (int b = 0; b < numOfBuilders; b++) {
        close(builderPipes[b][0]);
        close(builderPipes[b][1]);
        close(builderToRootPipes[b][1]); // Close write ends in root
        close(builderTimingPipes[b][1]);

    }


    // wait for all child processes to finish execution
    for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
        wait(NULL);
    }


    // PARENT PROCESS

    // CREATING THE MAIN HASHTABLE

    int wordsPerBuilder = (inputFileLines * 10) / numOfBuilders; // Approximately 10 words per line
    int mainTableCapacity = get_hash_table_capacity(wordsPerBuilder * numOfBuilders); // Scale for all builders
    struct hash_table *mainTable = create_hash_table(mainTableCapacity);
    
    for (int b = 0; b < numOfBuilders; b++) {
        close(builderToRootPipes[b][1]); // Close write ends in root

        while(1){
            int n;
            int freq;

            ssize_t nbytes = safe_read(builderToRootPipes[b][0], &n, sizeof(int));
            if(nbytes < 0){
                return 1;
            } else if(nbytes == 0){
                //EOF
                break;
            }
            if (n == 0) {
                // End marker received
                // printf("Root: Received end marker from Builder %d\n", b);
                break;
            }
            char *buffer = malloc(n);

            nbytes = safe_read(builderToRootPipes[b][0], buffer, sizeof(char) * n);
            if(nbytes < 0){
                return 1;
            } 
            nbytes = safe_read(builderToRootPipes[b][0], &freq, sizeof(int));
            if(nbytes < 0){
                return 1;
            } 

            struct hash_node *node = search_hash_table(mainTable, buffer);
            if(node != NULL){
                node->count += freq;
            } else{
                insert_hash_table_freq(mainTable, buffer, freq);
            }
            free(buffer);
        }

        close(builderToRootPipes[b][0]);
    }

    for (int b = 0; b < numOfBuilders; b++) {
        double elapsed_time;

        ssize_t nbytes = read(builderTimingPipes[b][0], &elapsed_time, sizeof(double));
        if (nbytes < 0) {
            perror("Error reading timing information");
            exit(1);
        } else if (nbytes == 0) {
            printf("No timing information received from Builder %d\n", b);
            continue;
        }

        printf("Builder %d took %f seconds to finish.\n", b, elapsed_time);

        close(builderTimingPipes[b][0]); // Close the read end of the timing pipe
    }
    // Find top-k

    int totalWords = 0;

    for(int i =0; i < mainTable->capacity; i++){
        struct hash_node *current_node = mainTable->array[i];
        while (current_node != NULL){
            totalWords++;
            current_node = current_node->next; 
        }
    }

    struct hash_node **word_array = malloc(totalWords * sizeof(struct hash_node*));
    if (word_array == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Populate the array
    int idx = 0;
    for (int i = 0; i < mainTable->capacity; i++) {
        struct hash_node *node = mainTable->array[i];
        while (node != NULL) {
            word_array[idx++] = node;
            node = node->next;
        }
    }

    qsort(word_array, totalWords, sizeof(struct hash_node *), compare_frequency);

    // print_hash_table(mainTable);

    FILE *filePtr = fopen("output.txt", "w"); 
    for (int i = 0; i < topK && i < totalWords; i++) {
        fprintf(filePtr,"%s: %d\n", word_array[i]->word, word_array[i]->count);
    }

    printf("Splitter signals: %d\n", completed_splitters);
    printf("Builder signals: %d\n", completed_builders);

    fclose(filePtr);
    free(word_array);
    destroy_hash_table(mainTable);
    free(exclusionList);
    return 0;
}