#ifndef BF_H
#define BF_H

#include "../lib/src/stack.h"

#define BF_DEFAULT_MEMORY 30000

const char *bf_lex(const char *program, int psize);
int bf_run(char *program, int psize, char *memory, int msize);

#endif
