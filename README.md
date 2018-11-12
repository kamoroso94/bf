# bf
A command line Brainfuck interpreter written in C.  The library functions
`bf_lex` and `bf_run` are available to you in `src/bf.c` and declared in
`src/bf.h`.

## Implementation
The Brainfuck program will operate on a fixed-sized, wrapping memory tape
(default of 30,000 bytes).  Each cell is a signed byte without integer overflow
or underflow protection.  The program will read from `stdin` and write to
`stdout`.  You can leverage file redirection on the command line to read from or
write to files.  When the program attempts to read beyond the end of the file,
`EOF` as defined in [`<stdio.h>`][1] will be returned.

## Compilation
Run `make` to compile the source with `gcc`.  You can use a different C compiler
by defining the `CC` environment variable and run `make` with the `-e` flag,
e.g. `CC=cc; make -e`.

## Usage
```
$ ./bf [-d dump_file] [-m memory_size] source_file
```
* `-d dump_file`: (optional) dump contents of program memory into `dump_file`
after execution.  Default is `/dev/null`.
* `-m memory_size`: (optional) allocate `memory_size` bytes of wrapping memory
for the program to use.  Default is 30,000.
* `source_file`: interpret `source_file` as Brainfuck source.

[1]: http://pubs.opengroup.org/onlinepubs/7908799/xsh/stdio.h.html
