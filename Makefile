# Compiler and flags
CC      = clang
CFLAGS  = -std=c23 -Wall -Wextra -Wpedantic -O0 -Werror

# Directories and target
SRC_DIR    = src
BUILD_DIR  = build
TARGET     = $(BUILD_DIR)/server

# Source and object files
SRC_FILES  = $(SRC_DIR)/server.c $(SRC_DIR)/response.c
OBJ_FILES  = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Default target: build the target executable
all: $(TARGET)

# Link object files into the target executable
$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ_FILES)

# Compile each C source file into an object file in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up generated files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
