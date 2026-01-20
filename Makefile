# =========================
# Makefile for Fluid Project
# =========================

# Compiler
CC = gcc

# Compilation flags
CFLAGS = -Wall -Wextra -O2 -Iinclude

# Linker flags (libraries)
LDLIBS = -lsqlite3 -lcurl -llzma -ltar

# Source and build directories
SRC_DIR = src
BUILD_DIR = build

# Find all .c source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Final binary
TARGET = $(BUILD_DIR)/fluid

# Default target
all: $(TARGET)

# Link object files into final binary
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

# Compile .c files into .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
