#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "bf.h"
#include "stack.h"

struct bracket {
  int token_idx;  // index of self in program
  int pair_idx;   // index of other in bracs
};

static int bf_parse(char *program, int psize, stack_t *brac_pairs);

// similar to strtok, set source string then call with NULL
// finds next bf command in program
// returns pointer to next token
const char *bf_lex(const char *program, int psize) {
  static int i = 0;
  static const char *p = NULL;
  char token;

  if(program != NULL) {
    i = 0;
    p = program;
  }
  if(p == NULL) return NULL;

  while(i < psize) {
    token = p[i++];
    if(strchr("><+-.,[]", token) != NULL) {
      return p + i - 1;
    }
  }

  return NULL;
}

// runs bf code in program with memory
// returns 0 on error
int bf_run(char *program, int psize, char *memory, int msize) {
  int result;
  int data_ptr = 0;
  int token_idx = 0;
  int bracket_idx = 0;
  stack_t brac_pairs;
  struct bracket *src, *dest, *bracs;

  if(!stack_create(&brac_pairs, sizeof(struct bracket))) {
    fprintf(stderr, "No memory\n");
    return 0;
  }

  // build table for bracket pairs and strip comments
  psize = bf_parse(program, psize, &brac_pairs);
  if(psize < 0) return 0;
  bracs = brac_pairs.buffer;

  while(token_idx < psize) {
    switch(program[token_idx]) {
      case '>':
      data_ptr++;
      if(data_ptr == msize) data_ptr = 0;
      break;

      case '<':
      if(data_ptr == 0) data_ptr = msize;
      data_ptr--;
      break;

      case '+':
      memory[data_ptr]++;
      break;

      case '-':
      memory[data_ptr]--;
      break;

      case '.':
      result = putchar(memory[data_ptr]);
      if(result == EOF) {
        perror(NULL);
        return 0;
      }
      break;

      case ',':
      errno = 0;
      result = getchar();
      if(result == EOF && errno != 0) {
        perror(NULL);
        return 0;
      }
      memory[data_ptr] = result;
      break;

      case '[':
      if(!memory[data_ptr]) {
        src = bracs + bracket_idx;
        dest = bracs + src->pair_idx;
        token_idx = dest->token_idx;
        bracket_idx = src->pair_idx;
      }
      bracket_idx++;
      break;

      case ']':
      src = bracs + bracket_idx;
      dest = bracs + src->pair_idx;
      token_idx = dest->token_idx - 1;  // re-enter loop for check
      bracket_idx = src->pair_idx;
      break;
    }
    token_idx++;
  }

  return 1;
}

// strips comments and builds table for bracket pairs
// returns -1 on error, final code length on success
static int bf_parse(char *program, int psize, stack_t *brac_pairs) {
  int token_idx = 0;
  int bracket_idx = 0;
  const char *token;
  stack_t bstack; // holds left brackets to pair with right brackets
  struct bracket l_brac, r_brac, *bracs;

  if(!stack_create(&bstack, sizeof(struct bracket))) {
    fprintf(stderr, "No memory\n");
    return -1;
  }

  token = bf_lex(program, psize);
  while(token != NULL) {
    switch(program[token_idx] = *token) {
      case '[': {
        // points to self
        l_brac.token_idx = token_idx;
        l_brac.pair_idx = bracket_idx;

        // make entry in bracket table and push to stack
        if(!stack_push(brac_pairs, &l_brac) || !stack_push(&bstack, &l_brac)) {
          stack_destroy(&bstack);
          fprintf(stderr, "No memory\n");
          return -1;
        }

        bracket_idx++;
        break;
      }

      case ']': {
        // syntax check
        if(bstack.length == 0) {
          stack_destroy(&bstack);
          fprintf(stderr, "Syntax Error: Unmatched ']'\n");
          return -1;
        }

        // points to self
        r_brac.token_idx = token_idx;
        r_brac.pair_idx = bracket_idx;

        // make entry in bracket table and pop from stack for pair
        if(!stack_push(brac_pairs, &r_brac) || !stack_pop(&bstack, &l_brac)) {
          stack_destroy(&bstack);
          fprintf(stderr, "No memory\n");
          return -1;
        }

        // make the pairs point to each other
        bracs = brac_pairs->buffer;
        bracs[l_brac.pair_idx].pair_idx = r_brac.pair_idx;
        bracs[r_brac.pair_idx].pair_idx = l_brac.pair_idx;

        bracket_idx++;
        break;
      }
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
