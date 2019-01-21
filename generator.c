#include <string.h>

#include "generator.h"

int run_operation(display_t *d, parser_meta_t *p, operation_t *op)
{
  switch (op->op)
    {
      
    case FORWARD:
      forward(d); break;

    case TURN:
      turn(d, op->param); break;
      
    case PUSH:
      save(&p->stack, d); break;
      
    case POP:
      restore(&p->stack, d); break;
      
    default:
      return 0;
    }

  return 1;
}

//
void eval(display_t *d, parser_meta_t *p, char *expr)
{
  int i = 0;

  while (expr[i])
    run_operation(d, p, &p->ops[expr[i++]]);
}

//
void generate1(display_t *d, parser_meta_t *p, int iter)
{
  int i, j, n;
  char *axiom    = malloc(sizeof(char) * BUFFER_SIZE);
  char *sentence = malloc(sizeof(char) * BUFFER_SIZE);

  memcpy(axiom, p->axiom, strlen(p->axiom));
  axiom[strlen(p->axiom)] = 0;

  init_stack(&p->stack, MAX_STACK_SIZE);
  
  n = 0;
  
  while (n < iter)
    {
      i = j = 0;

      while (axiom[i])
	{
	  if (p->rules[axiom[i]].state)
	    {
	      strncpy(sentence + j, p->rules[axiom[i]].rule, strlen(p->rules[axiom[i]].rule));
	      j += strlen(p->rules[axiom[i]].rule);
	    }
	  else
	    sentence[j++] = axiom[i];

	  i++;
	}

      strcpy(axiom, sentence);
      
      n++;
    }

  eval(d, p, axiom);

  free(axiom);
  free(sentence);
}

void generate0(display_t *d, parser_meta_t *p, int iter)
{
  int i, j, n;
  char *axiom    = malloc(sizeof(char) * BUFFER_SIZE);
  char *sentence = malloc(sizeof(char) * BUFFER_SIZE);

  memcpy(axiom, p->axiom, strlen(p->axiom));
  axiom[strlen(p->axiom)] = 0;

  init_stack(&p->stack, MAX_STACK_SIZE);
  
  n = 0;
  
  while (n < iter)
    {
      eval(d, p, axiom);

      i = j = 0;

      while (axiom[i])
	{
	  if (p->rules[axiom[i]].state)
	    {
	      strncpy(sentence + j, p->rules[axiom[i]].rule, strlen(p->rules[axiom[i]].rule));
	      j += strlen(p->rules[axiom[i]].rule);
	    }
	  else
	    sentence[j++] = axiom[i];

	  i++;
	}

      strcpy(axiom, sentence);
      
      n++;
    }
  
  free(axiom);
  free(sentence);
}

