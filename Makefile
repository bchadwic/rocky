# Makefile for compiling main.c with strict GCC warnings

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith \
         -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition \
         -Wunreachable-code -Wformat=2 -Winit-self -Wfloat-equal -Wundef -Wcast-qual \
         -Wwrite-strings -Wredundant-decls -Wnested-externs -Wno-unused-parameter \
         -std=gnu17 -O2

TARGET = odon
SRC = main.c recv.c send.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
