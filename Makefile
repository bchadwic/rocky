# Compiler
CC := gcc

# Output binary
TARGET := odon

# Source files
SRCS := main.c

# Object files
OBJS := $(SRCS:.c=.o)

# Default flags (debug build)
CFLAGS := -std=c11 -Wall -Wextra -g -O0
LDFLAGS := -lpthread

# Release flags
RELEASE_CFLAGS := -std=c11 -Wall -Wextra -O2 -flto -s
RELEASE_LDFLAGS := -flto -lpthread

# Default target
all: debug

# Debug build
debug: CFLAGS += -DDEBUG
debug: $(TARGET)

# Release build
release: CFLAGS := $(RELEASE_CFLAGS)
release: LDFLAGS := $(RELEASE_LDFLAGS)
release: $(TARGET)

# Link the binary
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
