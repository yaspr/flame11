#ifndef GENERATOR_H
#define GENERATOR_H

#include "stack.h"
#include "parser.h"
#include "display.h"

#define BUFFER_SIZE 100 * 1024 * 1024

void eval(display_t *d, parser_meta_t *p, char *expr);
void generate0(display_t *d, parser_meta_t *p, int iter);
void generate1(display_t *d, parser_meta_t *p, int iter);
int run_operation(display_t *d, parser_meta_t *p, operation_t *op);

#endif
