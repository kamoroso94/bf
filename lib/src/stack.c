#include <stdlib.h>
#include <string.h>
#include "stack.h"

int stack_create(stack_t *stack, int elsize) {
  int size = STACK_DEFAULT_LEN * elsize;
  stack->buffer = malloc(size);
  stack->length = 0;
  stack->size = size;
  stack->elsize = elsize;
  return stack->buffer != NULL;
}

int stack_push(stack_t *stack, const void *item) {
  void *buffer;

  if(stack->length * stack->elsize >= stack->size) {
    stack->size *= 2;
    buffer = realloc(stack->buffer, stack->size);
    if(buffer == NULL) return 0;
    stack->buffer = buffer;
  }

  memcpy(stack->buffer + stack->length * stack->elsize, item, stack->elsize);
  stack->length++;
  return 1;
}

int stack_pop(stack_t *stack, void *item) {
  void *buffer;

  if(stack->length == 0) return -1;

  stack->length--;
  memcpy(item, stack->buffer + stack->length * stack->elsize, stack->elsize);

  if(stack->length >= STACK_DEFAULT_LEN / 4 && stack->length * stack->elsize < stack->size / 4) {
    stack->size /= 4;
    buffer = realloc(stack->buffer, stack->size);
    if(buffer == NULL) return 0;
    stack->buffer = buffer;
  }

  return 1;
}

void stack_destroy(stack_t *stack) {
  free(stack->buffer);
}
