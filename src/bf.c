#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bf.h"
#include "stack.h"

struct bracket {
  int token_idx;  // index of self in program
  int pair_idx;   // index of other in bracs
};

static int bf_parse(char *program, int psize, stack_t *brac_tbl);
static int bf_parse_lbrac(int *token_idx, int *bracket_idx, stack_t *brac_tbl, stack_t *bstack);
static int bf_parse_rbrac(int *token_idx, int *bracket_idx, stack_t *brac_tbl, stack_t *bstack);
static void bf_cmd_right(int *data_ptr, int msize);
static void bf_cmd_left(int *data_ptr, int msize);
static void bf_cmd_inc(char *cell);
static void bf_cmd_dec(char *cell);
static int bf_cmd_out(char *cell);
static int bf_cmd_in(char *cell);
static void bf_cmd_lbrac(char *cell, int *token_idx, int *bracket_idx, struct bracket *bracs);
static void bf_cmd_rbrac(int *token_idx, int *bracket_idx, struct bracket *bracs);

// similar to strtok, set source string then call with NULL
// finds next bf command in program
// returns pointer to next token
const char *bf_lex(const char *program, int psize) {
  static int i = 0;
  static const char *p = NULL;
  char token;

  // reset lexer
  if(program != NULL) {
    i = 0;
    p = program;
  }
  if(p == NULL) return NULL;

  // find next token
  while(i < psize) {
    token = p[i++];
    if(strchr("><+-.,[]", token) != NULL) {
      return p + i - 1;
    }
  }

  return NULL;
}

// strips comments and builds table for bracket pairs
// returns -1 on error, final code length on success
static int bf_parse(char *program, int psize, stack_t *brac_tbl) {
  int token_idx = 0;
  int bracket_idx = 0;
  const char *token;
  stack_t bstack; // holds left brackets to pair with right brackets

  if(!stack_create(&bstack, sizeof(struct bracket))) {
    fprintf(stderr, "No memory\n");
    return -1;
  }

  // initialize lexer, build bracket table
  token = bf_lex(program, psize);
  while(token != NULL) {
    switch(program[token_idx] = *token) {
      case '[':
      if(!bf_parse_lbrac(&token_idx, &bracket_idx, brac_tbl, &bstack)) return -1;
      bracket_idx++;
      break;

      case ']':
      if(!bf_parse_rbrac(&token_idx, &bracket_idx, brac_tbl, &bstack)) return -1;
      bracket_idx++;
      break;
    }

    token = bf_lex(NULL, psize);
    token_idx++;
  }

  // syntax check
  if(bstack.length > 0) {
    stack_destroy(&bstack);
    fprintf(stderr, "Syntax Error: Unmatched '['\n");
    return -1;
  }

  stack_destroy(&bstack);
  return token_idx;
}

// make entry for left bracket and push left bracket onto stack
// returns 0 on error
static int bf_parse_lbrac(int *token_idx, int *bracket_idx, stack_t *brac_tbl, stack_t *bstack) {
  struct bracket l_brac;

  // points to self
  l_brac.token_idx = *token_idx;
  l_brac.pair_idx = *bracket_idx;

  // make entry in bracket table and push to stack
  if(!stack_push(brac_tbl, &l_brac) || !stack_push(bstack, &l_brac)) {
    stack_destroy(bstack);
    fprintf(stderr, "No memory\n");
    return 0;
  }

  return 1;
}

// make entry for right bracket and pop left bracket off of stack, pair them up
// returns 0 on error
static int bf_parse_rbrac(int *token_idx, int *bracket_idx, stack_t *brac_tbl, stack_t *bstack) {
  struct bracket l_brac, r_brac, *bracs;

  // syntax check
  if(bstack->length == 0) {
    stack_destroy(bstack);
    fprintf(stderr, "Syntax Error: Unmatched ']'\n");
    return 0;
  }

  // points to self
  r_brac.token_idx = *token_idx;
  r_brac.pair_idx = *bracket_idx;

  // make entry in bracket table and pop from stack for pair
  if(!stack_push(brac_tbl, &r_brac) || !stack_pop(bstack, &l_brac)) {
    stack_destroy(bstack);
    fprintf(stderr, "No memory\n");
    return 0;
  }

  // make the pairs point to each other
  bracs = brac_tbl->buffer;
  bracs[l_brac.pair_idx].pair_idx = r_brac.pair_idx;
  bracs[r_brac.pair_idx].pair_idx = l_brac.pair_idx;

  return 1;
}

// runs bf code in program with memory
// returns 0 on error
int bf_run(char *program, int psize, char *memory, int msize) {
  int data_ptr = 0;
  int token_idx = 0;
  int bracket_idx = 0;
  stack_t brac_tbl;

  if(!stack_create(&brac_tbl, sizeof(struct bracket))) {
    fprintf(stderr, "No memory\n");
    return 0;
  }

  // build table for bracket pairs and strip comments
  psize = bf_parse(program, psize, &brac_tbl);
  if(psize < 0) return 0;

  // execute bf command
  while(token_idx < psize) {
    switch(program[token_idx]) {
      case '>':
      bf_cmd_right(&data_ptr, msize);
      break;

      case '<':
      bf_cmd_left(&data_ptr, msize);
      break;

      case '+':
      bf_cmd_inc(memory + data_ptr);
      break;

      case '-':
      bf_cmd_dec(memory + data_ptr);
      break;

      case '.':
      if(!bf_cmd_out(memory + data_ptr)) return 0;
      break;

      case ',':
      if(!bf_cmd_in(memory + data_ptr)) return 0;
      break;

      case '[':
      bf_cmd_lbrac(memory + data_ptr, &token_idx, &bracket_idx, brac_tbl.buffer);
      break;

      case ']':
      bf_cmd_rbrac(&token_idx, &bracket_idx, brac_tbl.buffer);
      break;
    }
    token_idx++;
  }

  return 1;
}

// increment data pointer
static void bf_cmd_right(int *data_ptr, int msize) {
  (*data_ptr)++;
  if(*data_ptr == msize) *data_ptr = 0;
}

// decrement data pointer
static void bf_cmd_left(int *data_ptr, int msize) {
  if(*data_ptr == 0) *data_ptr = msize;
  (*data_ptr)--;
}

// increment byte at data pointer
static void bf_cmd_inc(char *cell) {
  (*cell)++;
}

// decrement byte at data pointer
static void bf_cmd_dec(char *cell) {
  (*cell)--;
}

// output byte at data pointer
// returns 0 on error
static int bf_cmd_out(char *cell) {
  int result = putchar(*cell);

  if(result == EOF) {
    perror(NULL);
    return 0;
  }

  return 1;
}

// input byte at data pointer
// returns 0 on error
static int bf_cmd_in(char *cell) {
  int result;
  errno = 0;
  result = getchar();

  if(result == EOF && errno != 0) {
    perror(NULL);
    return 0;
  }

  *cell = (char)result;
  return 1;
}

// enter loop or skip
static void bf_cmd_lbrac(char *cell, int *token_idx, int *bracket_idx, struct bracket *bracs) {
  struct bracket *src, *dest;

  if(!*cell) {
    // skip loop
    src = bracs + *bracket_idx;
    dest = bracs + src->pair_idx;
    *token_idx = dest->token_idx;
    *bracket_idx = src->pair_idx;
  }

  // count left (right if skipped) bracket
  (*bracket_idx)++;
}

// wrap back to matching left bracket
static void bf_cmd_rbrac(int *token_idx, int *bracket_idx, struct bracket *bracs) {
  struct bracket *src, *dest;

  // loop back
  src = bracs + *bracket_idx;
  dest = bracs + src->pair_idx;
  *token_idx = dest->token_idx - 1;  // re-enter loop for check
  *bracket_idx = src->pair_idx;
}
