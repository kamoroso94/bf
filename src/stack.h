#ifndef STACK_H
#define STACK_H

#define STACK_DEFAULT_LEN 16

typedef struct {
  void *buffer;
  int length;
  int size;
  int elsize;
} stack_t;

int stack_create(stack_t *stack, int elsize);
int stack_push(stack_t *stack, const void *item);
int stack_pop(stack_t *stack, void *item);
void stack_destroy(stack_t *stack);

#endif
