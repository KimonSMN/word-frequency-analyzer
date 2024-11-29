# Compiler
CC = gcc 

# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude

# Output executable name
TARGET = lexan

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Source files
SRC = $(SRC_DIR)/lexan.c $(SRC_DIR)/hashtable.c $(SRC_DIR)/splitter.c $(SRC_DIR)/builder.c $(SRC_DIR)/helper.c

# Object files
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = $(INCLUDE_DIR)hashtable.h $(INCLUDE_DIR)splitter.h $(INCLUDE_DIR)builder.h $(INCLUDE_DIR)helper.h

# Build the target executable
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile .c files into .o files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove object files and the executable
clean:
	rm -f $(TARGET) $(OBJ)

# PHONY targets
.PHONY: all clean
