# mytools

wc, head, tail, and grep -n written from scratch in C99, with speed
benchmarks against GNU coreutils.

## Build

Requires: `gcc` (or any C99 compiler), `make`, a POSIX environment
(Linux, macOS, WSL).

```bash
make
make debug
make clean
```

## Usage

Reads standard input and prints the line count.

```bash
./bin/mywc < file.txt
cat file.txt | ./bin/mywc
```

The command line is parsed in full: `-l`, `-w` and `-c`, bundled forms such
as `-lw`, `--` to end the options, `-` as a name for standard input, and file
operands. Only line counting is wired up so far, so every accepted flag
prints the same number and file operands are read but not opened.

A bad option is reported on stderr and exits with status 1, matching `wc`:

```bash
$ ./bin/mywc -x
./bin/mywc: invalid option -- 'x'
usage: ./bin/mywc [-lwc] [file...]
```

## Limitations

- Only line counting is implemented. `-w` and `-c` are accepted but have no effect yet, and file operands are parsed but never opened — input always comes from standard input.
- Options must come before file names. The first operand ends the options, so `mywc file -l` treats `-l` as a second file name. GNU `wc` permutes its arguments and would apply the flag; this follows the POSIX utility syntax guideline instead.
- Counts `\n` characters, not "lines". A file without a trailing newline gives a count one less — same as GNU `wc`, per the POSIX definition of a line.
- Counts bytes, not UTF-8 characters. Equivalent to `wc -c`, not `wc -m`.
- No buffer processing optimization — counting is byte by byte. See [Benchmark](#benchmark).

## Benchmark

- File: 76 MB (10,000,000 lines, `seq 1 10000000`)
- Measurement: best of 6 runs, page cache warm
- Machine: WSL2 Ubuntu 24.04

| version        | real   | user   | sys    |
|----------------|--------|--------|--------|
| mywc (64KB, byte-by-byte loop) | 0.091s | 0.085s | 0.004s |
| GNU wc 9.4     | 0.013s | 0.004s | 0.009s |

The cache is warm because a first measurement would be measuring disk while the second would measure RAM. The measurements show that my sys time and GNU's are approximately equal, meaning the buffering works and the number of system calls is the same.

The main difference is in user time, which means the main cause of the time difference is processing, not data delivery. GNU coreutils 9.4 is faster because it uses AVX2 instructions for `wc -l` (confirmed with `wc --debug`) which process a 32-byte block of data at once, while I call a function for every single byte in the buffer.

The plan is to replace the byte-by-byte loop with memchr over the whole buffer and measure again. I expect a speedup because memchr in glibc is SIMD-optimized.

### Effect of buffer size

Same counting code, the only change is the `BUFIO_SIZE` constant.
File: 5 MB (`head -c 5000000 /tmp/big.txt`), page cache warm.

| `BUFIO_SIZE` | `read()` calls | real   | sys    |
|--------------|----------------:|-------:|-------:|
| 1 B          |       5,000,001 | 0.705s | 0.562s |
| 64 KB        |              78 | 0.013s | 0.001s |

A 64 KB buffer reduces the number of system calls by ~64,000x, `sys`
time by ~560x, and total time by ~54x. The algorithm is unchanged —
the difference is entirely in how many times you cross into the kernel.

## What I learned
