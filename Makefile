# Makefile for Snake Game

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -D _DEFAULT_SOURCE

# Linker flags
LDFLAGS = -lncurses

# Target executable name
TARGET = claude-snake

# Source file
SRC = snake.c

# Object file
OBJ = $(SRC:.c=.o)

# Default installation directory
PREFIX = /usr/local/bin

# Default target
all: $(TARGET)

# Compile the game
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source to object file
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Install the game
install: $(TARGET)
	@mkdir -p $(PREFIX)
	install -m 755 $(TARGET) $(PREFIX)/$(TARGET)
	@echo "Snake game installed successfully to $(PREFIX)"

# Uninstall the game
uninstall:
	rm -f $(PREFIX)/$(TARGET)
	@echo "Snake game uninstalled from $(PREFIX)"

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJ)
	@echo "Cleaned up build files"

# Rebuild the project
rebuild: clean all

# Phony targets
.PHONY: all install uninstall clean rebuild
