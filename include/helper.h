#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>

ssize_t safe_read(int fd, void *buffer, size_t n);

void trim_newline(char *str);

void clean_string(char *str);

char *trimwhitespace(char *str);

int compare_frequency(const void *a, const void *b);
#endif