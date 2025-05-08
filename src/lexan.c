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

// Global variables for signal handling.
int completedSplitters = 0;
int completedBuilders = 0;

// Signal Handler
void handle_sigusr(int signal){
    if (signal == SIGUSR1) {
        completedSplitters++;
    } else if (signal == SIGUSR2) {
        completedBuilders++;
    }
}

int main(int argc, char *argv[]) {

    //////// HANDLE COMMAND LINE ARGUMENTS ////////

    char *textFile = NULL;          // Input File to be read.
    char *exclusionFile = NULL;     // Exclusion File.
    char *outputFile = NULL;        // Output File.
    int numOfSplitters = 0;         // Number of Splitters.
    int numOfBuilders = 0;          // Number of Builders.
    int topK = 0;                   // Number of Top-Popular Words.

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            textFile = argv[++i];
        } else if (strcmp(argv[i], "-l") == 0) {
            numOfSplitters = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0) {
            numOfBuilders = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0) {
            topK = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0) {
            exclusionFile = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFile = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }


    if (textFile == NULL || numOfSplitters < 1 || numOfBuilders < 1 || topK < 1 || exclusionFile == NULL || outputFile == NULL) {
        printf("Usage: ./lexan -i TextFile -l numOfSplitter -m numOfBuilders -t TopPopular -e ExclusionList -o OutputFile\n");
        exit(EXIT_FAILURE);
    }


    //////// HANDLE SIGNALS ////////
    struct sigaction sa;
    sa.sa_handler = &handle_sigusr;
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);


    // Process lines of Text File

    int textFileLines = count_lines(textFile);

   
    printf("Input File has: %d lines\n", textFileLines);

    // Process lines of Exclusion File

    int excFileLines = count_lines(exclusionFile);

    printf("Exclusion file has %d lines\n", excFileLines); // Debugging

   


    // //////// CREATE PIPES ////////
   
    // int builderPipes[numOfBuilders][2];         // Pipes for Splitters & Builders, [][0] for reading by builders, [][1] for writing by splitters
    // int builderToRootPipes[numOfBuilders][2];   // Pipes for Builders & Root,      [][0] for reading by root, [][1] for writing by builders
    // int builderTimingPipes[numOfBuilders][2];   // Pipes for Builder's time,       [][0] for reading by root, [][1] for writing by builders

    // for (int b = 0; b< numOfBuilders; b++) {
    //     if (pipe(builderPipes[b]) == -1
    //         || pipe(builderToRootPipes[b]) == -1
    //         || pipe(builderTimingPipes[b]) == -1) { 

    //         perror("Error: Pipe creation failed");
    //         exit(1);
    //     }
    // }

    // //////// FORK SPLITTERS ////////

    // for (int s = 0; s < numOfSplitters; s++) {
    //     int pid = fork();
    //     if (pid == -1) { // Error checking.
    //         perror("Error: Splitter creation failed.");
    //         exit(1);
    //     }
    //     if (pid == 0) { // Child process (splitter).
        
    //         for (int b = 0; b < numOfBuilders;b++){ 
    //             close(builderPipes[b][0]); // Close read ends.
    //         }

    //         splitter(s, numOfSplitters, numOfBuilders, textFile, textFileLines, builderPipes, exclusionFileLines, exclusionList);

    //         for (int b = 0; b < numOfBuilders; b++) {
    //             close(builderPipes[b][1]); // Close write ends.
    //         }

    //         return 1;   // Exit after processing.
    //     }
    // }

    // //////// FORK BUILDERS ////////

    // for (int b = 0; b < numOfBuilders; b++) {
    //     int pid = fork();
    //     if (pid == -1) {
    //         perror("Error: Builder creation failed.");
    //         exit(1);
    //     }

    //     if (pid == 0) { // Child process

    //         for (int i = 0; i < numOfBuilders; i++) {
    //             close(builderPipes[i][1]); // Close write ends
    //             if (i != b) {
    //                 close(builderPipes[i][0]); // Close read ends of other builders
    //             }
    //             close(builderToRootPipes[i][0]); // Close read ends of builderToRootPipes
    //             if (i != b) {
    //                 close(builderToRootPipes[i][1]); // Close write ends of other builders
    //             }
    //         }

    //         builder(b, numOfBuilders, builderPipes, builderToRootPipes, textFileLines, builderTimingPipes);

    //         close(builderPipes[b][0]);
    //         close(builderToRootPipes[b][1]);

    //         return 1;   // Exit after processing.
    //     }
    // }

    // // Close all pipe ends.
    // for (int b = 0; b < numOfBuilders; b++) {
    //     close(builderPipes[b][0]);
    //     close(builderPipes[b][1]);
    //     close(builderToRootPipes[b][1]);    // Close only write, we still use read end.
    //     close(builderTimingPipes[b][1]);    // Close only write, we still use read end.
    // }

    // // Wait for all child processes to finish execution.
    // for (int i = 0; i < numOfSplitters + numOfBuilders; i++) {
    //     wait(NULL);
    // }

    // //////// INITIALIZE HASHTABLE ////////

    // int wordsPerBuilder = (textFileLines * 10) / numOfBuilders; // Approximately 10 words per line.
    // int mainTableCapacity = get_hash_table_capacity(wordsPerBuilder * numOfBuilders);   // This function finds "good" hashtable sizes.
    // struct hash_table *mainTable = create_hash_table(mainTableCapacity);
    
    // for (int b = 0; b < numOfBuilders; b++) {
    //     close(builderToRootPipes[b][1]);

    //     while(1){
    //         int n, freq;

    //         ssize_t result = safe_read(builderToRootPipes[b][0], &n, sizeof(int));
    //         if(result < 0){
    //             return 1;
    //         } else if(result == 0){
    //             //EOF
    //             break;
    //         }

    //         char *buffer = malloc(n);

    //         result = safe_read(builderToRootPipes[b][0], buffer, sizeof(char) * n);
    //         if(result < 0){
    //             return 1;
    //         } 
    //         result = safe_read(builderToRootPipes[b][0], &freq, sizeof(int));
    //         if(result < 0){
    //             return 1;
    //         } 

    //         struct hash_node *node = search_hash_table(mainTable, buffer);
    //         if(node != NULL){
    //             node->count += freq;
    //         } else{
    //             insert_hash_table_freq(mainTable, buffer, freq);
    //         }
    //         free(buffer);
    //     }

    //     close(builderToRootPipes[b][0]);
    // }

    // for (int b = 0; b < numOfBuilders; b++) {
    //     double elapsed_time;

    //     ssize_t result = read(builderTimingPipes[b][0], &elapsed_time, sizeof(double));
    //     if (result < 0) {
    //         perror("Error: Reading time information failed.");
    //         exit(1);
    //     } else if (result == 0) {
    //         printf("No time information received from Builder: %d\n", b);
    //         continue;
    //     }

    //     printf("Builder: %d took: %f seconds to finish.\n", b, elapsed_time);

    //     close(builderTimingPipes[b][0]); // Close the read end of the timing pipe
    // }

    // // Find top-k
    // int totalWords = 0;

    // for(int i =0; i < mainTable->capacity; i++){
    //     struct hash_node *current_node = mainTable->array[i];
    //     while (current_node != NULL){
    //         totalWords++;
    //         current_node = current_node->next; 
    //     }
    // }

    // struct hash_node **word_array = malloc(totalWords * sizeof(struct hash_node*));
    // if (word_array == NULL) {
    //     perror("Malloc failed");
    //     exit(EXIT_FAILURE);
    // }

    // int idx = 0;
    // for (int i = 0; i < mainTable->capacity; i++) {
    //     struct hash_node *node = mainTable->array[i];
    //     while (node != NULL) {
    //         word_array[idx++] = node;
    //         node = node->next;
    //     }
    // }

    // qsort(word_array, totalWords, sizeof(struct hash_node *), compare_frequency);

    // FILE *filePtr = fopen(outputFile, "w"); // Open output file for writing.
    // for (int i = 0; i < topK && i < totalWords; i++) {
    //     fprintf(filePtr,"%s: %d\n", word_array[i]->word, word_array[i]->count); // Write top-k words to output file.
    // }

    // printf("Splitter signals received: %d\n", completedSplitters);
    // printf("Builder signals received: %d\n", completedBuilders);

    // fclose(filePtr);    // Close output file.

    // free(word_array);
    // free(exclusionList);
    // destroy_hash_table(mainTable);

    return 0;
}