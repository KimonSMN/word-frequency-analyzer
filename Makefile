# Makefile for compiling the pipe communication program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = 

# Output executable name
TARGET = lexan

# Source files
SRC = lexan.c hashtable.c

HEADERS = hashtable.h

# Build target
all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# PHONY targets
.PHONY: all clean
