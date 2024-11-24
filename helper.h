#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>

ssize_t read_nbytes(int fd, void *buffer, size_t n);
ssize_t write_nbytes(int fd, const void *buffer, size_t n);


#endif