# Compiler
CC = gcc

# Compiler flags
CFLAGS =  

# Output executable name
TARGET = lexan

# Source files
SRC = lexan.c hashtable.c splitter.c builder.c helper.c

# Object files (replace .c with .o)
OBJ = $(SRC:.c=.o)

# Header files
HEADERS = hashtable.h splitter.h builder.h helper.h

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
