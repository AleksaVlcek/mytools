CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O2 -Iinclude -D_POSIX_C_SOURCE=200809L

all: bin/mywc

bin/mywc: src/wc.c include/bufio.h src/bufio.c
	mkdir -p bin
	$(CC) $(CFLAGS) src/wc.c src/bufio.c -o bin/mywc

debug: src/wc.c include/bufio.h src/bufio.c
	mkdir -p bin
	$(CC) $(CFLAGS) src/wc.c src/bufio.c -g -O0 -fsanitize=address,undefined -o bin/mywc-debug

clean:
	rm -rf bin build

.PHONY: all clean
