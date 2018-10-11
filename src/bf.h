#include "stack.h"

#ifndef BF_H
#define BF_H

#define BF_DEFAULT_MEMORY 4096

typedef struct {
  int flags;
  int in_fd;
  int out_fd;
  int dump_fd;
  char *program;
  int prgm_size;
  char *memory;
  int mem_size;
  stack_t *brac_tbl;
} bf_params_t;

typedef struct {
  int token_idx;
  int bracket_idx;
} bracket_t;

#endif
