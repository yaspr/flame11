#ifndef PARSER_H
#define PARSER_H

#include "stack.h"

#define MAX_ALPHA     256
#define MAX_RULES     256
#define MAX_RULE_LEN  1024
#define MAX_ENTRIES   1024

#define FORWARD  0 //Move forward
#define TURN     1 //turn
#define PUSH     2 //push current (x, y, angle) on the stack
#define POP      3 //pop
#define PUSHT    4 //push and turn
#define POPT     5 //pop and turn

#define ERR_ORIG_RAD   0
#define ERR_ORIG_ALPHA 1
#define ERR_ORIG_RULES 2
#define ERR_ORIG_DEFS  3
#define ERR_ORIG_PARSE 4

#define NB_ERRORS 5
#define ERR_MSG_LEN 64

//
static const char errors[NB_ERRORS][ERR_MSG_LEN] = { "rad angle format error"   ,
						     "alphabet format error"    ,
						     "rules format error"       ,
						     "definitions format error" ,
						     "parsing error" };

static int curr = 0;

/*
  ALPHABET; RULE; RULES; ; DEFINITIONS;
  
  ALPHABET    = { SYMBOL [, SYMBOL]* };
  RULE        = SYMBOL [SYMBOL]*;
  RULES       = (SYMBOL, RULE) [, (SYMBOL, RULE)]*;
  DEFINITIONS = (SYMBOL, OPERATION) [, (SYMBOL, OPERATION)]*;
  
  SYMBOL = characters
  OPERATION = forward | [+ | -]NUMBER * PI / NUMBER]])
  
 */

//
typedef struct operation_s { char state; char op; double param; } operation_t;

//
typedef struct rule_s { char state; char rule[MAX_RULE_LEN]; } rule_t;

//
typedef struct parser_meta_s {
                                 char        alphabet[MAX_ALPHA];
                                 char        axiom[MAX_RULE_LEN];
                                 rule_t      rules[MAX_RULES];
                                 operation_t ops[MAX_ALPHA];
                                 int nb_ops;
  
                                 stack_t stack;
  
                             } parser_meta_t;

//
typedef struct parser_entry_s {
                                 int id;
                                 parser_meta_t grammar;
                              } parser_entry_t;

//
parser_meta_t *load_file(char *fname);
void print_parser_meta(parser_meta_t *p);

#endif
