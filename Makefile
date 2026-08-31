CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O2 -Iinclude -D_POSIX_C_SOURCE=200809L
DEPFLAGS = -MMD -MP
DBGFLAGS = -g -O0 -fsanitize=address,undefined

BUILD = build
TOOLS = wc
COMMON = bufio

COMMON_O = $(COMMON:%=$(BUILD)/%.o)
BINS = $(TOOLS:%=bin/my%)
TOOL_O = $(TOOLS:%=$(BUILD)/%.o)

BUILD_D = $(BUILD)/debug
COMMON_DO = $(COMMON:%=$(BUILD_D)/%.o)
TOOL_DO = $(TOOLS:%=$(BUILD_D)/%.o)
BINS_D = $(TOOLS:%=bin/debug/my%)

OBJS = $(COMMON_O) $(TOOL_O) $(COMMON_DO) $(TOOL_DO)
DEPS = $(OBJS:.o=.d)

all: $(BINS)

$(BUILD):
	mkdir -p $@

bin:
	mkdir -p $@

$(BUILD_D):
	mkdir -p $@

bin/debug:
	mkdir -p $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

bin/my%: $(BUILD)/%.o $(COMMON_O) | bin
	$(CC) $(CFLAGS) $^ -o $@

debug: $(BINS_D)

$(BUILD_D)/%.o: src/%.c | $(BUILD_D)
	$(CC) $(CFLAGS) $(DBGFLAGS) $(DEPFLAGS) -c $< -o $@

bin/debug/my%: $(BUILD_D)/%.o $(COMMON_DO) | bin/debug
	$(CC) $(CFLAGS) $(DBGFLAGS) $^ -o $@

test: all debug
	./tests/run.sh

clean:
	rm -rf bin build

-include $(DEPS)

.PHONY: all clean debug test

# Without this, make deletes the .o files as intermediates and every build is a full rebuild
.SECONDARY: $(OBJS)
