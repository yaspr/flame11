#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi.h"

#include "parser.h"

//
void error(int err)
{
  printf("#Error (%d): %s\n", curr, errors[err]);
  exit(-1);
}

//deg = a * PI / b
double conv_rad2deg(int a, int b)
{ return a * PI / b; }

//PI --> 180 deg.
double conv_deg2rad(double angle)
{ return angle * 180 / PI; }

//
int is_alpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

//
int is_digit(char c)
{
  return (c >= '0' && c <= '9');
}

//
int to_digit(char c)
{
  return c - '0';
}

//Move over spaces, tabs, and new lines
int walk(FILE *fd)
{
  char c;
  int i = 0;
  
  while ((c = fgetc(fd)) != EOF && (c == ' ' || c == '\t' || c == '\n'))
    i++;

  fseek(fd, -1, SEEK_CUR);

  curr += i;

  return i;
}

//
int get_number(FILE *fd)
{
  int val = 0;
  char c = fgetc(fd);
  
  while (is_digit(c))
    {
      val *= 10;
      val += to_digit(c);

      c = fgetc(fd);

      curr++;
    }

  fseek(fd, -1, SEEK_CUR);
  
  return val;
}

//
double get_rad_angle(FILE *fd, int *a, int *b)
{
  char c, d;
  
  (*a) = get_number(fd);

  walk(fd);

  c = fgetc(fd); curr++;

  if (c == '*')
    {
      walk(fd);

      c = fgetc(fd); curr++;
      d = fgetc(fd); curr++;
      
      if ((c == 'P' || c == 'p') && (d == 'I' || d == 'i'))
	{
	  walk(fd);
	  
	  c = fgetc(fd); curr++;

	  if (c == '/')
	    {
	      walk(fd);

	      (*b) = get_number(fd);
	    }
	  else
	    error(ERR_ORIG_RAD);
	}
      else
	error(ERR_ORIG_RAD);
    }
  else
    error(ERR_ORIG_RAD);

  return (*a) * PI / (*b);
}

//
char get_symbol(FILE *fd)
{ 
  curr++;
  return fgetc(fd); 
}

//
int get_string(FILE *fd, char *str)
{
  int i = 0;
  char c = fgetc(fd);
  
  while (is_alpha(c))
    {
      str[i++] = c;

      c = fgetc(fd); curr++;
    }

  str[i] = 0;
  
  fseek(fd, -1, SEEK_CUR);

  return i;
}

//
int get_alphabet(FILE *fd, parser_meta_t *p)
{
  char c;
  int i = 0;
  
  c = fgetc(fd); curr++;

  if (c == '{')
    {
      walk(fd);

      p->alphabet[i++] = get_symbol(fd);
      
      walk(fd);

      c = fgetc(fd); curr++;
      
      while (c == ',')
	{
	  walk(fd);
	  p->alphabet[i++] = get_symbol(fd);
	  walk(fd);
	  
	  c = fgetc(fd); curr++;
	}

      //
      p->alphabet[i] = 0;
      
      if (c != '}')
	error(ERR_ORIG_ALPHA);
    }
  else
    error(ERR_ORIG_ALPHA);

  return 0;
}

//
int symbol_in(char *set, char s)
{
  int found = 0, i = 0;
  
  while (!found && set[i])
    found = (s == set[i++]);
  
  return found;
}

//
int get_rule_string(FILE *fd, parser_meta_t *p, char *dst)
{
  char c;
  int i = 0;
  
  c = fgetc(fd); curr++;

  while (symbol_in(p->alphabet, c))
    {
      dst[i++] = c;
      
      c = fgetc(fd); curr++;
    }
  
  fseek(fd, -1, SEEK_CUR);
  
  dst[i] = 0;

  return 0;
}

//
int get_rule(FILE *fd, parser_meta_t *p, char *dst)
{
  char tmp[MAX_RULE_LEN];
  
  get_rule_string(fd, p, tmp);

  strncpy(dst, tmp, strlen(tmp));
  
  return 0;
}

//
int get_axiom(FILE *fd, parser_meta_t *p)
{
  return get_rule(fd, p, p->axiom);
}

//
int get_rules(FILE *fd, parser_meta_t *p)
{
  char c, d, s;
  
  c = fgetc(fd); curr++;

  while (c == '(')
    {
      walk(fd);

      s = get_symbol(fd);

      walk(fd);
      
      d = fgetc(fd); curr++;

      if (d == ',')
	{
	  walk(fd);

	  get_rule(fd, p, p->rules[s].rule);
	  p->rules[s].state = 1;
	  
	  walk(fd);

	  d = fgetc(fd); curr++;

	  if (d != ')')
	    error(ERR_ORIG_RULES);

	  walk(fd);

	  c = fgetc(fd); curr++;
	}
      else
	error(ERR_ORIG_RULES);
    }

  fseek(fd, -1, SEEK_CUR);
  
  return 0;
}

//
int get_definitions(FILE *fd, parser_meta_t *p)
{
  int a, b;
  char c, d, s;
  double theta;
  char tmp[MAX_RULE_LEN];
  
  c = fgetc(fd);

  while (c == '(')
    {
      walk(fd);

      s = get_symbol(fd);
      
      walk(fd);

      d = fgetc(fd); curr++;

      if (d == ',')
	{
	  walk(fd);

	  d = fgetc(fd); curr++;
	  	  
	  if (d == '+' || d == '-')
	    {
	      theta = get_rad_angle(fd, &a, &b);
	      
	      theta = (d == '-') ? -theta : theta;
	      
	      p->ops[s].state = 1;
	      p->ops[s].op = TURN;
	      p->ops[s].param = theta;
	    }
	  else
	    {
	      fseek(fd, -1, SEEK_CUR);

	      get_string(fd, tmp);

	      if (!strncmp(tmp, "forward", 7))
		{
		  p->ops[s].state = 1;
		  p->ops[s].op = FORWARD;
		}
	      else
		if (!strncmp(tmp, "push", 4))
		  {
		    p->ops[s].state = 1;
		    p->ops[s].op = PUSH;
		  }
		else
		  if (!strncmp(tmp, "pop", 3))
		    {
		      p->ops[s].state = 1;
		      p->ops[s].op = POP;
		    }
		  else
		    error(ERR_ORIG_DEFS);
	    }
	  
	  walk(fd);
	  
	  d = fgetc(fd); curr++;
	  
	  if (d != ')')
	    error(ERR_ORIG_DEFS);
	  
	  walk(fd);
	  
	  c = fgetc(fd); curr++;
	}
      else
	error(ERR_ORIG_DEFS);
    }

  fseek(fd, -1, SEEK_CUR);

  return 0;
}

//
int parse(FILE *fd, parser_meta_t *p)
{
  char c;
  
  get_alphabet(fd, p);

  walk(fd);
  
  c = fgetc(fd); curr++;

  if (c == ';')
    {
      walk(fd);
      
      get_axiom(fd, p);

      walk(fd);

      c = fgetc(fd); curr++;
      
      if (c == ';')
	{
	  walk(fd);

	  get_rules(fd, p);

	  walk(fd);

	  c = fgetc(fd); curr++;
	  
	  if (c == ';')
	    {
	      walk(fd);

	      get_definitions(fd, p);

	      walk(fd);

	      c = fgetc(fd); curr++;
	      
	      if (c != ';')
		error(ERR_ORIG_PARSE);
	    }
	  else
	    error(ERR_ORIG_PARSE);
	}
      else
	error(ERR_ORIG_PARSE);
    }
  else
    error(ERR_ORIG_PARSE);

  return 0;
}

//
parser_meta_t *load_file(char *fname)
{
  FILE *fd = fopen(fname, "rb");

  if (fd)
    {
      parser_meta_t *p = malloc(sizeof(parser_meta_t));

      walk(fd);
      
      parse(fd, p);
      
      fclose(fd);
      
      return p;
    }
  else
    return NULL;
}

void print_parser_meta(parser_meta_t *p)
{
  int i = 0;
  
  printf("Alphabet: { %s }\n", p->alphabet);
  
  printf("Axiom: %s\n", p->axiom);

  i = 0;

  while (p->alphabet[i])
    {
      if (p->rules[p->alphabet[i]].state)
	printf("%c\t\t%s\n", p->alphabet[i], p->rules[p->alphabet[i]].rule);

      i++;
    }

  i = 0;
  
  while (p->alphabet[i])
    {
      if (p->ops[p->alphabet[i]].state)
	if (p->ops[p->alphabet[i]].op == FORWARD)
	  printf("%c\t\tforward\n", p->alphabet[i]);
	else
	  if (p->ops[p->alphabet[i]].op == TURN)
	    printf("%c\t\tturn\t\t%lf\n", p->alphabet[i], p->ops[p->alphabet[i]].param);
	  else
	    if (p->ops[p->alphabet[i]].op == PUSH)
	      printf("%c\t\tpush\n", p->alphabet[i]);
	    else
	      if (p->ops[p->alphabet[i]].op == POP)
		printf("%c\t\tpop\n", p->alphabet[i]);

      i++;
    }
  
  
}
