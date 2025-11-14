CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic \
           -Wshadow -Wpointer-arith -Wcast-qual \
           -Wstrict-prototypes -Wmissing-prototypes \
           -Wconversion -Wuninitialized -Wunreachable-code \
           -Wfloat-equal -Wwrite-strings -Wswitch-enum \
           -Wredundant-decls -Wformat=2 -Wno-discarded-qualifiers

SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:.c=.o)
TARGET  := odon
LIB     := libodon.a

.PHONY: all clean unit integration

all: $(TARGET)

# Build static library from core sources
$(LIB): $(OBJ)
	ar rcs $@ $^

# Build main executable using the library
$(TARGET): $(LIB) main.o
	$(CC) $(CFLAGS) -o $@ main.o $(LIB)

# Generic object rule
%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

clean:
	rm -f $(OBJ) main.o $(TARGET) $(LIB) $(TEST_BIN)

# ---------- Unit testing ----------

TEST_SRC := $(wildcard test/unit/*.c)
TEST_BIN := $(TEST_SRC:.c=)

unit: $(TEST_BIN)
	@for t in $(TEST_BIN); do \
		echo "Running $$t"; \
		./$$t || exit 1; \
	done

# Build test executables linking against the core library
test/unit/%: test/unit/%.c $(LIB)
	$(CC) $(CFLAGS) -Iinclude -o $@ $< $(LIB)

# ---------- Integration tests (Go) ----------

integration: all
	cd test/integration && go test -v .
