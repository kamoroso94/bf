#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "bf.h"
#include "stack.h"

#define D_FLAG 0x1
#define M_FLAG 0x2
#define O_FLAG 0x4
#define SYS_WARN (perror(NULL))
#define MEM_WARN (fprintf(stderr, "No memory\n"))
#define SYS_ERROR (SYS_WARN, exit(EXIT_FAILURE))
#define MEM_ERROR (MEM_WARN, exit(EXIT_FAILURE))

static void print_usage(const char *prgm_name) {
  printf("usage: %s [-d dump_file] [-m memory_size] [-o output_file] input_file\n", prgm_name);
  exit(EXIT_FAILURE);
}

static void parse_args(int argc, char **argv, bf_params_t *params);
static void destroy_params(bf_params_t *params);
static const char *bf_lex(const char *program, int size);
static int bf_parse(char *program, int psize, stack_t *table);
static int bf_run(bf_params_t *params);

int main(int argc, char *argv[]) {
  int exit_code;
  bf_params_t params;
  struct stat sb;
  stack_t table;

  params.brac_tbl = &table;
  parse_args(argc, argv, &params);

  if(fstat(params.in_fd, &sb) == -1) {
    close(params.in_fd);
    SYS_ERROR;
  }

  params.program = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, params.in_fd, 0);
  if(params.program == MAP_FAILED) {
    close(params.in_fd);
    SYS_ERROR;
  }

  if(!stack_create(&table, sizeof(bracket_t))) {
    munmap(params.program, sb.st_size);
    close(params.in_fd);
    MEM_ERROR;
  }

  params.prgm_size = bf_parse(params.program, sb.st_size, &table);
  if(params.prgm_size == -1) {
    stack_destroy(&table);
    munmap(params.program, sb.st_size);
    close(params.in_fd);
    exit(EXIT_FAILURE);
  }

  params.memory = calloc(params.mem_size, sizeof(char));
  if(params.memory == NULL) {
    stack_destroy(&table);
    munmap(params.program, sb.st_size);
    close(params.in_fd);
    MEM_ERROR;
  }

  exit_code = bf_run(&params) ? EXIT_SUCCESS : EXIT_FAILURE;

  stack_destroy(&table);
  free(params.memory);
  munmap(params.program, sb.st_size);
  destroy_params(&params);

  return exit_code;
}

static void parse_args(int argc, char **argv, bf_params_t *params) {
  int opt;
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  if(argc < 2) print_usage(argv[0]);

  params->flags = 0;
  params->mem_size = BF_DEFAULT_MEMORY;
  params->dump_fd = -1;
  params->out_fd = STDOUT_FILENO;

  while((opt = getopt(argc, argv, "d:m:o:")) != -1) {
    switch(opt) {
      case 'd':
      if(params->flags & D_FLAG) break;
      params->flags |= D_FLAG;
      params->dump_fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, mode);
      if(params->dump_fd == -1) {
        destroy_params(params);
        SYS_ERROR;
      }
      break;

      case 'm':
      if(params->flags & M_FLAG) break;
      params->flags |= M_FLAG;
      params->mem_size = atoi(optarg);
      if(params->mem_size <= 0) params->mem_size = BF_DEFAULT_MEMORY;
      break;

      case 'o':
      if(params->flags & O_FLAG) break;
      params->flags |= O_FLAG;
      params->out_fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, mode);
      if(params->out_fd == -1) {
        destroy_params(params);
        SYS_ERROR;
      }
      break;
    }
  }

  if(argv[optind] == NULL) print_usage(argv[0]);
  params->in_fd = open(argv[optind], O_RDWR);
  if(params->in_fd == -1) {
    destroy_params(params);
    SYS_ERROR;
  }
}

static void destroy_params(bf_params_t *params) {
  if(params->flags & D_FLAG) close(params->dump_fd);
  if(params->flags & O_FLAG) close(params->out_fd);
}

static int bf_run(bf_params_t *params) {
  int result;
  int data_ptr = 0;
  bracket_t *src, *dest;
  bracket_t *bracs = params->brac_tbl->buffer;
  bracket_t state = {
    .token_idx = 0,
    .bracket_idx = 0
  };
  
  // printf("Running...\n");

  while(state.token_idx < params->prgm_size) {
    switch(params->program[state.token_idx]) {
      case '>':
      data_ptr++;
      if(data_ptr == params->mem_size) data_ptr = 0;
      break;

      case '<':
      if(data_ptr == 0) data_ptr = params->mem_size;
      data_ptr--;
      break;

      case '+':
      params->memory[data_ptr]++;
      break;

      case '-':
      params->memory[data_ptr]--;
      break;

      case '.':
      result = write(params->out_fd, params->memory + data_ptr, 1);
      if(result == -1) {
        SYS_WARN;
        return 0;
      }
      break;

      case ',':
      params->memory[data_ptr] = getchar();
      break;

      case '[':
      if(!params->memory[data_ptr]) {
        src = bracs + state.bracket_idx;
        dest = bracs + src->bracket_idx;
        state.token_idx = dest->token_idx;
        state.bracket_idx = src->bracket_idx;
      }
      state.bracket_idx++;
      break;

      case ']':
      src = bracs + state.bracket_idx;
      dest = bracs + src->bracket_idx;
      state.token_idx = dest->token_idx - 1;
      state.bracket_idx = src->bracket_idx;
      break;
    }
    state.token_idx++;
  }

  if(params->flags & D_FLAG) {
    result = write(params->dump_fd, params->memory, params->mem_size);
    if(result == -1) {
      SYS_WARN;
      return 0;
    }
  }
  
  return 1;
}

static int bf_parse(char *program, int psize, stack_t *table) {
  const char *token;
  stack_t bstack;
  bracket_t l_brac, r_brac, *bracs;
  bracket_t state = {
    .token_idx = 0,
    .bracket_idx = 0
  };
  
  // printf("Parsing...\n");

  if(!stack_create(&bstack, sizeof(bracket_t))) {
    MEM_WARN;
    return -1;
  }

  while((token = bf_lex(program, psize)) != NULL) {
    program[state.token_idx] = *token;
    switch(*token) {
      case '[': {
        l_brac.token_idx = state.token_idx;
        l_brac.bracket_idx = state.bracket_idx;

        if(!stack_push(table, &l_brac) || !stack_push(&bstack, &l_brac)) {
          stack_destroy(&bstack);
          MEM_WARN;
          return -1;
        }

        state.bracket_idx++;
        break;
      }

      case ']': {
        if(bstack.length == 0) {
          stack_destroy(&bstack);
          fprintf(stderr, "Syntax Error: Unmatched ']'\n");
          return -1;
        }

        r_brac.token_idx = state.token_idx;
        r_brac.bracket_idx = state.bracket_idx;

        if(!stack_push(table, &r_brac) || !stack_pop(&bstack, &l_brac)) {
          stack_destroy(&bstack);
          MEM_WARN;
          return -1;
        }

        bracs = table->buffer;
        bracs[l_brac.bracket_idx].bracket_idx = r_brac.bracket_idx;
        bracs[r_brac.bracket_idx].bracket_idx = l_brac.bracket_idx;

        state.bracket_idx++;
        break;
      }
    }
    state.token_idx++;
  }

  if(bstack.length > 0) {
    stack_destroy(&bstack);
    fprintf(stderr, "Syntax Error: Unmatched '['\n");
    return -1;
  }

  stack_destroy(&bstack);

  return state.token_idx;
}

static const char *bf_lex(const char *program, int psize) {
  static int i = 0;
  char token;

  while(i < psize) {
    token = program[i++];
    if(strchr("><+-.,[]", token) != NULL) {
      return program + i - 1;
    }
  }

  return NULL;
}
