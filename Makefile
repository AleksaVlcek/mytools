CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O2 -Iinclude -D_POSIX_C_SOURCE=200809L

all: bin/mywc

bin/mywc: src/wc.c
	mkdir -p bin
	$(CC) $(CFLAGS) src/wc.c -o bin/mywc

clean:
	rm -rf bin build

.PHONY: all clean
