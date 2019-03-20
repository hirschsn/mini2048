
.SUFFIXES:
.SUFFIXES: .c

CFLAGS = -Wall -Wextra -Os
LDFLAGS = -static

BIN = mini2048
SRC = mini2048.c

all: $(BIN)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm $(BIN)

.PHONY: clean all

