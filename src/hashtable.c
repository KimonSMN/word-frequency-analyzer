#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hashtable.h"
#include "helper.h"


int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};


// Hash function
unsigned long hash(unsigned char *str)
{
    if (str == NULL || *str == '\0') {
        return 0;
    }
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

struct hash_table *create_hash_table(int capacity){
    struct hash_table *table = malloc(sizeof(struct hash_table));
    if (table == NULL){     // Error checking.
        perror("Error: Memory allocation for hash table failed.");
        exit(1);
    }
    table->capacity = capacity;
    if (capacity <= 0) {    // Error checking.
        perror("Error: Capacity is <= than 0.");
        exit(1);
    }
    table->array = calloc(capacity, sizeof(struct hash_node*));
    if(table->array == NULL){   // Error checking.
        perror("Error: Memory allocation for hash table array failed.");
        exit(1);
    }
    return table;
}

void insert_hash_table(struct hash_table *table, char *word) {
    if (table == NULL || word == NULL) { // Error checking.
        perror("Error: Tried to insert to NULL table OR NULL word");
        exit(1);
    }
    unsigned long index = hash((unsigned char *)word) % table->capacity;

    struct hash_node *node = table->array[index];
    while (node != NULL) {
        if (strcmp(node->word, word) == 0) {
            node->count++;
            return;
        }
        node = node->next;
    }

    struct hash_node *new_node = malloc(sizeof(struct hash_node));
    if (new_node == NULL){  // Error checking.
        perror("Error: Memory allocation for new node failed.");
        exit(1);
    }
    new_node->word = strdup(word);
    if(new_node->word == NULL){ // Error checking.
        perror("Error: Duplication of word failed.");
        free(new_node);
        exit(1);
    }
    new_node->count = 1;
    new_node->next = table->array[index];
    table->array[index] = new_node;
}

void insert_hash_table_freq(struct hash_table *table, char *word, int freq) {
    if (table == NULL || word == NULL) {    // Error checking.
        perror("Error: Tried to insert to NULL table OR NULL word");
        exit(1);
    }
    unsigned long index = hash((unsigned char *)word) % table->capacity;

    struct hash_node *node = table->array[index];
    while (node != NULL) {
        if (strcmp(node->word, word) == 0) {
            node->count++;
            return;
        }
        node = node->next;
    }

    struct hash_node *new_node = malloc(sizeof(struct hash_node));
    if (new_node == NULL){  // Error checking.
        perror("Error: Memory allocation for new node failed.");
        exit(1);
    }
    new_node->word = strdup(word);
    if(new_node->word == NULL){ // Error checking.
        perror("Error: Duplication of word failed.");
        free(new_node);
        exit(1);
    }
    new_node->count = freq;
    new_node->next = table->array[index];
    table->array[index] = new_node;
}


struct hash_node *search_hash_table(struct hash_table *table, char *word){
    if (table == NULL || word == NULL) {    // Error checking.
        perror("Error: Tried to search NULL table OR NULL word");
        exit(1);
    }
    unsigned long index = hash((unsigned char *)word) % table->capacity;

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
    if (table == NULL) {    // Error checking.
        return;
    }
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
    if (table == NULL) {    // Error checking.
        perror("Error: Table is empty.");
        exit(1);
    }
    for (int i = 0; i < table->capacity; i++){
        struct hash_node *node = table->array[i];
        while(node != NULL){

            int n = strlen(node->word) + 1;
            if(write(writeFd ,&n, sizeof(int)) <= 0){ 
                perror("Error writing word length");// Error checking.
                exit(1);
            }

            if(write(writeFd, node->word, sizeof(char) * n) <= 0){
                perror("Error writing word");       // Error checking.
                exit(1);
            }
            
            if(write(writeFd, &node->count, sizeof(int)) <= 0){
                perror("Error writing frequency");  // Error checking.
                exit(1);
            }
            node = node->next;
        }
    }
    int endMarker = 0;
    if (write(writeFd, &endMarker, sizeof(int)) < 0) {
        perror("Error writing end marker"); // Error checking.
        exit(1);
    }
}

int get_hash_table_capacity(int wordsPerBuilder){
    int len = sizeof(prime_sizes) / sizeof(int);
    for (int i = 0; i < len; i++){
        if(prime_sizes[i] > wordsPerBuilder){
            return prime_sizes[i];
        }
    }
    return prime_sizes[len - 1];
}
