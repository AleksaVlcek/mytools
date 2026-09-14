# mytools

wc, head, tail, and grep -n written from scratch in C99, with speed
benchmarks against GNU coreutils.

## Build

Requires: `gcc` (or any C99 compiler), `make`, a POSIX environment
(Linux, macOS, WSL).

```bash
make          # bin/mywc
make debug    # bin/debug/mywc, built with ASan and UBSan
make clean
```

## Usage

```bash
./bin/mywc [-l] [-w] [-c] [file...]
```

Counts lines, words and bytes. With no flag it prints all three; the columns
always come in that order, whatever order the flags were given in. With no
file operand it reads standard input, and `-` as an operand means standard
input as well. `--` ends the options, so a file whose name starts with a dash
can be reached with `mywc -l -- -weird`.

Several files each get a row, plus a `total` row, with the columns aligned:

```bash
$ ./bin/mywc -l tests/data/normal.txt tests/data/binary.bin
     3 tests/data/normal.txt
   393 tests/data/binary.bin
   396 total
```

A file that cannot be read is reported on stderr, the rest are still counted,
and the exit status is 1:

```bash
$ ./bin/mywc -l nema_me tests/data/normal.txt; echo $?
./bin/mywc: nema_me: No such file or directory
3 tests/data/normal.txt
3 total
1
```

An unknown option prints the reason and the usage line and also exits 1.

## Limitations

- Options must come before file names. The first operand ends the options, so `mywc file -l` treats `-l` as a second file name. GNU `wc` permutes its arguments and would apply the flag; this follows the POSIX utility syntax guideline instead.
- Words are counted over bytes: a word is a run of bytes that `isspace()` rejects, so only ASCII whitespace separates words. GNU `wc` decodes the input in the current locale, so in a UTF-8 locale it also splits on Unicode spaces such as U+00A0 and U+2003, and its count on input that is not text depends on the locale. Measured with GNU coreutils 9.11 on `tests/data/binary.bin`: 2279 words from `mywc`, 2279 from `wc` under `en_US.UTF-8`, 2658 under `LC_ALL=C`. On plain ASCII text they agree.
- Counts `\n` characters, not "lines". A file without a trailing newline gives a count one less — same as GNU `wc`, per the POSIX definition of a line.
- Counts bytes, not UTF-8 characters. `-c` matches `wc -c`; there is no `-m`.
- `-c` always reads the whole input. GNU takes the size from `fstat` when the input is a regular file and never reads it, which is why `wc -c` on a large file returns instantly.
- No buffer processing optimization — counting is byte by byte. See [Benchmark](#benchmark).

## Benchmark

Measured when `mywc` only counted lines from standard input. Counting words
and bytes adds work per byte, so these numbers are a floor rather than the
current figure; they will be taken again alongside the `memchr` rewrite.

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
