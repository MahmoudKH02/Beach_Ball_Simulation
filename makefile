# Compiler and flags
CC = gcc
CFLAGS = -Wall
LDFLAGS_DRAWER = -lm -lglut -lGLU -lGL

# Object file directory
OBJ_DIR = obj

# Source files for each process
PROCESS_SOURCES = player.c teamLead.c main.c drawer.c

# Common object file for shared functions
FUNCTIONS_OBJECT = $(OBJ_DIR)/functions.o

# Executables (excluding drawer)
EXECUTABLES = $(basename $(PROCESS_SOURCES))

# Default target (build all executables)
all: $(EXECUTABLES)

# Rule to compile each process source file into object file
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile functions.c into functions.o (shared functions)
$(FUNCTIONS_OBJECT): functions.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


# Rule to link each executable (except drawer) with common LDFLAGS
$(MATH_USERS): %: $(OBJ_DIR)/%.o $(FUNCTIONS_OBJECT)
	$(CC) $^ -o $@ $(MATH_LIB)


# Rule to link each executable (except drawer) with common LDFLAGS
$(filter-out $(MATH_USERS) drawer,$(EXECUTABLES)): %: $(OBJ_DIR)/%.o $(FUNCTIONS_OBJECT)
	$(CC) $^ -o $@


# Rule to link drawer executable with specific LDFLAGS
drawer: $(OBJ_DIR)/drawer.o $(FUNCTIONS_OBJECT)
	$(CC) $^ -o $@ $(LDFLAGS_DRAWER)


# Create object file directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean target (remove object files and executables)
clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLES)

# Phony targets
.PHONY: all clean
