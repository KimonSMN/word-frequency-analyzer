# Makefile for compiling the pipe communication program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = 

# Output executable name
TARGET = splitter

# Source files
SRC = splitter.c hashtable.c

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
