#include "stack.h"

#ifndef BF_H
#define BF_H

#define BF_DEFAULT_MEMORY 4096

const char *bf_lex(const char *program, int psize);
int bf_run(char *program, int psize, char *memory, int msize);

#endif
