# Compiler
CC = gcc 

# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
# Source files
SRC = $(SRC_DIR)/lexan.c \
      $(SRC_DIR)/hashtable.c \
      $(SRC_DIR)/splitter.c \
      $(SRC_DIR)/builder.c \
      $(SRC_DIR)/helper.c

# Object files
OBJECTS = $(BUILD_DIR)/lexan.o \
          $(BUILD_DIR)/hashtable.o \
          $(BUILD_DIR)/splitter.o \
          $(BUILD_DIR)/builder.o \
          $(BUILD_DIR)/helper.o

# Output executable name
TARGET = $(BIN_DIR)/lexan
 
# Header files
HEADERS = $(INCLUDE_DIR)hashtable.h $(INCLUDE_DIR)splitter.h $(INCLUDE_DIR)builder.h $(INCLUDE_DIR)helper.h

# Build the target executable
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	
# Clean target to remove object files and the executable
clean:
	rm -f $(TARGET) $(OBJECTS)

# PHONY targets
.PHONY: all clean
