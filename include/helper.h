#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>

#define BUFFER_SIZE 8192 

ssize_t safe_read(int fd, void *buffer, size_t n);

void trim_newline(char *str);

void clean_string(char *str);

char *trim_space(char *str);

int compare_frequency(const void *a, const void *b);


int count_lines(const char* filename);


#endif