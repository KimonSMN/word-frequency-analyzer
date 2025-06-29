# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
BIN_DIR = bin

# Source files
COMMON_SRC = $(SRC_DIR)/hashtable.c \
             $(SRC_DIR)/helper.c

LEXAN_SRC = $(SRC_DIR)/lexan.c
SPLITTER_SRC = $(SRC_DIR)/splitter.c
BUILDER_SRC = $(SRC_DIR)/builder.c

# Object files
COMMON_OBJ = $(COMMON_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
LEXAN_OBJ = $(BUILD_DIR)/lexan.o
SPLITTER_OBJ = $(BUILD_DIR)/splitter.o
BUILDER_OBJ = $(BUILD_DIR)/builder.o

# Executables
LEXAN_EXE = $(BIN_DIR)/lexan
SPLITTER_EXE = $(BIN_DIR)/splitter
BUILDER_EXE = $(BIN_DIR)/builder

# Default target
all: $(LEXAN_EXE) $(SPLITTER_EXE) $(BUILDER_EXE)

# Executable rules
$(LEXAN_EXE): $(COMMON_OBJ) $(LEXAN_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(SPLITTER_EXE): $(COMMON_OBJ) $(SPLITTER_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDER_EXE): $(COMMON_OBJ) $(BUILDER_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
