#include "stack.h"

//
int init_stack(stack_t *s, int max)
{
  s->sp = -1;
  s->MAX_STACK = max;
}

//
int save(stack_t *s, display_t *d)
{
  if (s->sp < s->MAX_STACK)
    {
      s->sp++;

      s->stack_x_curr[s->sp]     = d->x_curr;
      s->stack_y_curr[s->sp]     = d->y_curr;
      
      s->stack_angle[s->sp] = d->angle;

      /* printf("save: %lf %lf %lf\n", d->x_curr, d->y_curr, d->angle); */
      
      return 1;
    }
  else
    return 0;
}

//
int restore(stack_t *s, display_t *d)
{
  if (s->sp >= 0)
    {
      d->x_curr     = s->stack_x_curr[s->sp];
      d->y_curr     = s->stack_y_curr[s->sp];      
      
      d->angle = s->stack_angle[s->sp];

      /* printf("restore: %lf %lf %lf\n", d->x_curr, d->y_curr, d->angle); */
      
      s->sp--;
      
      return 1;
    }
  else
    return 0;
}
