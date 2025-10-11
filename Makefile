CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude
LDFLAGS :=

SRC := src/stun.c src/stun_pack.c main.c
OBJ := $(SRC:.c=.o)

TARGET := rocky

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
