#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "bf.h"

struct bf_params {
  int prgm_fd;
  int dump_fd;
  char *program;
  int prgm_size;
  char *memory;
  int mem_size;
};

static void print_usage(const char *prgm_name);
static int parse_args(int argc, char **argv, struct bf_params *params);
static int parse_d_arg(int *flags, struct bf_params *params);
static int parse_m_arg(int *flags, struct bf_params *params);
static int parse_src_arg(char **argv, struct bf_params *params);

int main(int argc, char *argv[]) {
  int result, exit_code;
  struct bf_params params;
  struct stat sb;

  // parse command line arguments, open source/dump files
  if(!parse_args(argc, argv, &params)) {
    exit(EXIT_FAILURE);
  }

  // get program source size
  if(fstat(params.prgm_fd, &sb) == -1) {
    close(params.prgm_fd);
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  params.prgm_size = sb.st_size;

  // open program source
  params.program = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, params.prgm_fd, 0);
  if(params.program == MAP_FAILED) {
    close(params.prgm_fd);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  // allocate program memory
  params.memory = calloc(params.mem_size, sizeof(char));
  if(params.memory == NULL) {
    munmap(params.program, sb.st_size);
    close(params.prgm_fd);
    fprintf(stderr, "No memory\n");
    exit(EXIT_FAILURE);
  }

  // run bf program
  exit_code = EXIT_SUCCESS;
  if(!bf_run(params.program, params.prgm_size, params.memory, params.mem_size)) {
    exit_code = EXIT_FAILURE;
  }

  // output contents of program memory
  if(params.dump_fd != -1) {
    result = write(params.dump_fd, params.memory, params.mem_size);
    if(result == -1) {
      perror(NULL);
      exit_code = EXIT_FAILURE;
    }
  }

  // clean up
  free(params.memory);
  munmap(params.program, sb.st_size);
  close(params.prgm_fd);
  if(params.dump_fd != -1) close(params.dump_fd);

  return exit_code;
}

// prints program command line usage
static void print_usage(const char *prgm_name) {
  printf("usage: %s [-d dump_file] [-m memory_size] source_file\n", prgm_name);
}

// parse command line arguments
// returns 0 on error
static int parse_args(int argc, char **argv, struct bf_params *params) {
  int opt, result;
  int flags = 0;

  if(argc < 2) {
    print_usage(argv[0]);
    return 0;
  }

  params->mem_size = BF_DEFAULT_MEMORY;
  params->dump_fd = -1;

  // parse command line flags
  while((opt = getopt(argc, argv, "d:m:")) != -1) {
    switch(opt) {
      case 'd':
      result = parse_d_arg(&flags, params);
      if(result == 0) return 0;
      if(result < 0) break;
      break;

      case 'm':
      result = parse_m_arg(&flags, params);
      if(result < 0) break;
      break;
    }
  }

  return parse_src_arg(argv, params);
}

// open dump file for program memory
// returns -1 on break, 0 on error, 1 on success
static int parse_d_arg(int *flags, struct bf_params *params) {
  const int D_FLAG = 0x1;
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

  if(*flags & D_FLAG) return -1;
  *flags |= D_FLAG;

  // open dump file
  params->dump_fd = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, mode);
  if(params->dump_fd == -1) {
    perror(NULL);
    return 0;
  }

  return 1;
}

// set program memory size, or default if invalid
// returns -1 on break, 1 on success
static int parse_m_arg(int *flags, struct bf_params *params) {
  const int M_FLAG = 0x2;

  if(*flags & M_FLAG) return -1;
  *flags |= M_FLAG;

  // set memory size
  params->mem_size = atoi(optarg);
  if(params->mem_size <= 0) params->mem_size = BF_DEFAULT_MEMORY;

  return 1;
}

// opens program source file
// returns 0 on error
static int parse_src_arg(char **argv, struct bf_params *params) {
  if(argv[optind] == NULL) {
    print_usage(argv[0]);
    return 0;
  }

  // open source file
  params->prgm_fd = open(argv[optind], O_RDWR);
  if(params->prgm_fd == -1) {
    if(params->dump_fd != -1) close(params->dump_fd);
    perror(NULL);
    return 0;
  }

  return 1;
}
