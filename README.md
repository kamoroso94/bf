# `bf`
A command line Brainfuck interpreter written in C.
The library functions `bf_lex` and `bf_run` are available to you in `bf.c` and decalred in `bf.h`.

## Compilation
Run `make` to compile the source with GCC.  You can redefine the `CC` variable in the Makefile to your desired C compiler.

## Usage
```
$ ./bf [-d dump_file] [-m memory_size] source_file [< input_file] [> output_file]
```
* `-d dump_file`: (optional) dump contents of program memory into `dump_file` after execution.  Default is `/dev/null`.
* `-m memory_size`: (optional) allocate `memory_size` bytes of wrapping memory for the program to use.  Default is 4096.
* `source_file`: interpret `source_file` as Brainfuck source.
* `input_file`: (optional) the `,` command will read from this file.  Default is `stdin`.
* `output_file`: (optional) the `.` command will write to this file.  Default is `stdout`.
