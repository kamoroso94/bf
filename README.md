# `bf`
A command line Brainfuck interpreter written in C.

## Compilation
Run `make` to compile the source with GCC.  You can redefine the `CC` variable in the Makefile to your desired C compiler.

## Usage
```
$ ./bf [-d dump_file] [-m memory_size] [-o output_file] source_file
```
* `-d dump_file`: (optional) dump contents of program memory into `dump_file` after execution.  Default is `/dev/null`.
* `-m memory_size`: (optional) allocate `memory_size` bytes of wrapping memory for the program to use.  Default is 4096.
* `-o output_file`: (optional) write output of program to `output_file`.  Default is `stdout`.
* `source_file`: interpret `source_file` as Brainfuck source.
