CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic \
           -Wshadow -Wpointer-arith -Wcast-qual \
           -Wstrict-prototypes -Wmissing-prototypes \
           -Wconversion -Wuninitialized -Wunreachable-code \
           -Wfloat-equal -Wwrite-strings -Wswitch-enum \
           -Wredundant-decls -Wformat=2 -Wno-discarded-qualifiers
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o) main.o
TARGET  := odon

WIN_CC     := i686-w64-mingw32-gcc
WIN_CFLAGS := -std=c99 -Wall -O2 -static

.PHONY: all clean test windows

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

windows: $(SRC)
	$(WIN_CC) $(WIN_CFLAGS) -o $(TARGET).exe $(SRC)

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

test: all
	cd test && go test -v .
