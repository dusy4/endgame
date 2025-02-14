CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -w
LDFLAGS = -lraylib -lm

SRC_DIR = src
BUILD_DIR = build
RES_DIR = resources
TARGET = game

SRC = $(SRC_DIR)/main.c
OBJ = $(BUILD_DIR)/main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run

