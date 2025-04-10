# Compiler and flags
CC      = clang
CFLAGS  = -std=c23 -Wall -Wextra -Wpedantic -O0 -Werror

# Directories and target
SRC_DIR    = src
BUILD_DIR  = build
TARGET     = $(BUILD_DIR)/server

# Default target
all: $(TARGET)

# Build the target executable from server.c
$(TARGET): $(SRC_DIR)/server.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_DIR)/server.c

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up generated files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean
