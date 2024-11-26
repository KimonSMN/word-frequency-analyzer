#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hashtable.h"
#include "helper.h"
// Hash function
unsigned long hash(unsigned char *str, int capacity)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % capacity;
}

struct hash_table *create_hash_table(int capacity){
    struct hash_table *table = malloc(sizeof(struct hash_table));
    table->capacity = capacity;
    table->array = calloc(capacity, sizeof(struct hash_node*));    
    return table;
}

void insert_hash_table(struct hash_table *table, char *word) {

    unsigned long index = hash(word, table->capacity);

    struct hash_node *node = table->array[index];
    while (node != NULL) {
        if (strcmp(node->word, word) == 0) {
            node->count++;
            return;
        }
        node = node->next;
    }

    struct hash_node *new_node = malloc(sizeof(struct hash_node));
    new_node->word = strdup(word);
    new_node->count = 1;
    new_node->next = table->array[index];
    table->array[index] = new_node;
}

void insert_hash_table_freq(struct hash_table *table, char *word, int freq) {

    unsigned long index = hash(word, table->capacity);

    struct hash_node *node = table->array[index];
    while (node != NULL) {
        if (strcmp(node->word, word) == 0) {
            node->count++;
            return;
        }
        node = node->next;
    }

    struct hash_node *new_node = malloc(sizeof(struct hash_node));
    new_node->word = strdup(word);
    new_node->count = freq;
    new_node->next = table->array[index];
    table->array[index] = new_node;
}


struct hash_node *search_hash_table(struct hash_table *table, char *word){
    unsigned long index = hash(word, table->capacity);

    struct hash_node *node_to_search = table->array[index];
    while (node_to_search != NULL){
        if(strcmp(node_to_search->word, word) == 0){
            return node_to_search;
        }
        node_to_search = node_to_search->next;
    }
    return NULL;
}

void destroy_hash_table(struct hash_table *table){
    for (int i = 0; i < table->capacity; i++) {
        struct hash_node *current_node = table->array[i];

        while (current_node != NULL) {
            struct hash_node *temp_node = current_node;
            current_node = current_node->next;
            free(temp_node->word);
            free(temp_node);
        }
    }
    free(table->array);
    free(table);
}

void print_hash_table(struct hash_table *table) {

    for (int i = 0; i < table->capacity; i++) {
        struct hash_node *node = table->array[i];  

        if (node != NULL) {
            printf("Bucket %d:\n", i);
        }

        while (node != NULL) {
            printf("\tWord: %s Frequency: %d\n", node->word, node->count);
            
            node = node->next;
            
        }
    }
}


void send_hash_table_to_root(struct hash_table *table, int writeFd){
    for (int i = 0; i < table->capacity; i++){
        struct hash_node *node = table->array[i];
        while(node != NULL){
            printf("Builder is sending word: %s, frequency: %d\n", node->word, node->count);

            int n = strlen(node->word) + 1;
            if(write(writeFd ,&n, sizeof(int)) != sizeof(int)){
                perror("Error writing word length");
                exit(EXIT_FAILURE);
            }

            if(write(writeFd, node->word, sizeof(char) * n) != n){
                perror("Error writing word");
                exit(EXIT_FAILURE);
            }
            
            if(write(writeFd, &node->count, sizeof(int)) != sizeof(int)){
                perror("Error writing frequency");
                exit(EXIT_FAILURE);
            }

            node = node->next;
        }
    }
    int end_marker = 0;
    if (write(writeFd, &end_marker, sizeof(int)) != sizeof(int)) {
        perror("Error writing end marker");
        exit(1);
    }
    
}

