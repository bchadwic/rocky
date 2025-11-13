CC      := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic \
          -Wshadow -Wpointer-arith -Wcast-qual \
          -Wstrict-prototypes -Wmissing-prototypes \
          -Wconversion -Wuninitialized -Wunreachable-code \
          -Wfloat-equal -Wwrite-strings -Wswitch-enum \
          -Wredundant-decls -Wformat=2 -Wno-discarded-qualifiers
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o) main.o
TARGET  := odon

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

test: all
	cd test && go test -v .
