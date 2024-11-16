# Makefile for compiling the pipe communication program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall 

# Output executable name
TARGET = lexan

# Source files
SRC = lexan.c

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# PHONY targets
.PHONY: all clean
