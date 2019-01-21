#ifndef STACK_H
#define STACK_H

#include "display.h"

#define MAX_STACK_SIZE 4096 //

//
typedef struct stack_s { double stack_x_curr[MAX_STACK_SIZE];
                         double stack_y_curr[MAX_STACK_SIZE];
                         double stack_angle[MAX_STACK_SIZE];
  
                         int sp;
                         int MAX_STACK; } stack_t;

int init_stack(stack_t *s, int max);
int save(stack_t *s, display_t *d);
int restore(stack_t *s, display_t *d);
void release_stack(stack_t *s);

#endif
