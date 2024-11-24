#ifndef HASHTABLE_H
#define HASHTABLE_H

struct hash_table { // Hash-table
    struct hash_node **array; // Πίνακας απο pointers σε hash_nodes.
    int capacity;             // Αριθμός των buckets (μέγεθος του hash-table).
};

struct hash_node {  // Κόμβος του hash-table
    char *word;              // Key is a word of the text.
    int count;              // Value is the count of the word.
    struct hash_node *next; // Pointer στον επόμενο hash_node στο ίδιο bucket. // Seperate Chaining
};

unsigned long hash(unsigned char *str, int capacity);

struct hash_table *create_hash_table(int capacity);

void insert_hash_table(struct hash_table *table, char *word);

struct hash_node *search_hash_table(struct hash_table *table, char *word);

void destroy_hash_table(struct hash_table *table);

void print_hash_table(struct hash_table *table);

#endif