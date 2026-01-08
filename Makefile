# Compiler and Flags
CC := gcc
CFLAGS := -O2 -Wall -Wextra -g

# Folders
SRC_DIR := src
BUILD_DIR := build

# Output Program
TARGET := $(BUILD_DIR)/install_nano

# All C files in $(SRC_DIR)
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Objects
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Rules
all: $(BUILD_DIR) $(TARGET)

# Cretae build folder
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create executable rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Compile objects rule
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

# Installation
install: $(TARGET)
	cp install_nano /usr/bin/

.PHONY: all clean
