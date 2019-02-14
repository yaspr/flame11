#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "flame.h"

#include "pi.h"

#define G 4

#define ORIG_X 400
#define ORIG_Y 200

//
typedef unsigned char byte;

//
typedef struct xy_s { double x; double y; } xy_t;

//
typedef struct double_pend_s
{
  //
  double prev_p1_x;
  double prev_p1_y;

  double p1_x;
  double p1_y;
  double r1;
  double m1;    
  double len1;
  double a1;

  double vel1;
  
  double acc1;

  //
  double prev_p2_x;
  double prev_p2_y;
  
  double p2_x;
  double p2_y;
  double r2;
  double m2;
  double len2;
  double a2;

  double vel2;
  
  double acc2;
  
  double g;
  
} double_pend_t;

//
static inline int randxy(int a, int b)
{ return (rand() % (b - a + 1)) + a; }

//
static inline double dist(double x1, double y1, double x2, double y2)
{
  double x = x1 - x2;
  double y = y1 - y2;
  
  return sqrt(x * x + y * y);
}

//
static inline void norm(double *x, double *y)
{
  double d = dist(*x, *y, 0, 0);
  
  *x /= d;
  *y /= d;
}

//
static inline void scale(double *x, double *y, double s)
{
  *x *= s;
  *y *= s;
}

//
static inline void limit(double *x, double *y, double s)
{
  norm(x, y);
  scale(x, y, s);
}

//
void draw_ball(flame_obj_t *fo, double x, double y, double px, double py, double w, double r)
{
  flame_set_color(fo, 0, 0, 0);
  for (double a = 0.0; a < 2 * PI; a += 0.1)
    flame_draw_point(fo, px + (r + w / 2) * cos(a), py + (r + w / 2) * sin(a));

  flame_set_color(fo, 255, 255, 255);

  for (double a = 0.0; a < 2 * PI; a += 0.1)
    flame_draw_point(fo, x + (r + w / 2) * cos(a), y + (r + w / 2) * sin(a));
}

//
void draw_cord1(flame_obj_t *fo, double_pend_t *p)
{
  //Remove previous line
  flame_set_color(fo, 0, 0, 0);
  flame_draw_line(fo, ORIG_X, ORIG_Y, p->prev_p1_x, p->prev_p1_y);
  
  flame_set_color(fo, 255, 255, 255);
  flame_draw_line(fo, ORIG_X, ORIG_Y, p->p1_x, p->p1_y);
}  

//
void draw_cord2(flame_obj_t *fo, double_pend_t *p)
{
  //Remove previous line
  flame_set_color(fo, 0, 0, 0);
  flame_draw_line(fo, p->prev_p1_x, p->prev_p1_y, p->prev_p2_x, p->prev_p2_y);
  
  flame_set_color(fo, 255, 255, 255);
  flame_draw_line(fo, p->p1_x, p->p1_y, p->p2_x, p->p2_y);
}  

//
void init_double_pend(double_pend_t *p)
{
  p->prev_p1_x = 0;
  p->prev_p1_x = 0;

  p->prev_p2_x = 0;
  p->prev_p2_x = 0;

  //First section
  p->r1   = 5;
  p->m1   = 25; 
  p->len1 = 100;
  p->a1 = PI / 4;

  p->p1_x = ORIG_X + p->len1 * sin(p->a1);
  p->p1_y = ORIG_Y + p->len1 * cos(p->a1);

  p->vel1 = 0;

  p->acc1 = 0.1;

  //Second section
  p->r2   = 5;
  p->m2   = 20; 
  p->len2 = 100;
  p->a2 = PI / 8;
  
  p->p2_x = p->p1_x + p->len2 * sin(p->a2);
  p->p2_y = p->p1_y + p->len2 * cos(p->a2);
  
  p->vel2 = 0;

  p->acc2 = -0.001;
  
}

//
void update(double_pend_t *p)
{
  p->prev_p1_x = p->p1_x;
  p->prev_p1_y = p->p1_y;

  p->prev_p2_x = p->p2_x;
  p->prev_p2_y = p->p2_y;

  //Rotating Spring
  /* p->a1   += p->vel1; */
  /* p->vel1 += p->acc1; */

  //Rotating Spring
  /* p->a2   += p->vel2; */
  /* p->vel2 += p->acc2; */

  //Acceleration equations to simulate a double pendulum
  
  double num1, num2, num3, num4, den;
  double a1, a2;
  
  num1 = -G * (2 * p->m1 + p->m2) * sin(p->a1);
  num2 = -p->m2 * G * sin(p->a1 - 2 * p->a2);
  num3 = -2 * sin(p->a1 - p->a2) * p->m2;
  num4 = p->vel2 * p->vel2 * p->len2 + p->vel1 * p->vel1 * p->len1 * sin(p->a1 - p->a2);
  den  = p->len1 * (2 * p->m1 + p->m2 - p->m2 * cos(2 * p->a1 - 2 * p->a2));
  
  a1   = (num1 + num2 + num3 * num4) / den;

  p->acc1 = a1;
    
  num1 = 2 * sin(p->a1 - p->a2);
  num2 = p->vel1 * p->vel1 * p->len1 * (p->m1 + p->m2);
  num3 = G * (p->m1 + p->m2) * cos(p->a1);
  num4 = p->vel2 * p->vel2 * p->len2 * cos(p->a1 - p->a2);
  den  = p->len2 * (2 * p->m1 + p->m2 - p->m2 * cos(2 * p->a1 - 2 * p->a2));

  a2   = num1 * (num2 + num3 + num4) / den;

  p->acc2 = a2;
  
  p->p1_x = ORIG_X + p->len1 * sin(p->a1);
  p->p1_y = ORIG_Y + p->len1 * cos(p->a1);
  
  p->p2_x = p->p1_x + p->len2 * sin(p->a2);
  p->p2_y = p->p1_y + p->len2 * cos(p->a2);

  p->vel1 += p->acc1;
  p->vel2 += p->acc2;
	  
  p->a1   += p->vel1;
  p->a2   += p->vel2;  
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int prev_click_x, prev_click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  flame_obj_t *fo = flame_open("Bounce", x_max, y_max);

  srand(getpid());

  XEvent event;
  flame_clear_display(fo);

  double_pend_t p;

 run:
  
  init_double_pend(&p);
  
  //
  while (1)
    {
      if (XPending(fo->display) > 0)
	{
	  XNextEvent(fo->display, &event);
	  
	  if (event.type == KeyPress)
	    {
	      c = XLookupKeysym(&event.xkey, 0);
	      
	      if (c == 'q')
		break;
	    }
	  else
	    if (event.type == ButtonPress)
	      {
		prev_click_x = click_x;
		prev_click_y = click_y;

		click_x = event.xkey.x;
		click_y = event.xkey.y;		  
	      }
	}
      else
	{	  
	  draw_cord1(fo, &p);
	  draw_ball(fo, p.p1_x, p.p1_y, p.prev_p1_x, p.prev_p1_y, p.m1, p.r1);
	  
	  draw_cord2(fo, &p);
	  draw_ball(fo, p.p2_x, p.p2_y, p.prev_p2_x, p.prev_p2_y, p.m2, p.r2);
	  
	  update(&p);

	  flame_set_color(fo, 0, 0, 255);
	  flame_draw_point(fo, p.p1_x, 50 + p.p1_y);
	  flame_set_color(fo, 0, 255, 0);
	  flame_draw_point(fo, p.p2_x, 100 + p.p2_y);
	  
	  usleep(100000);
	}
    }

  flame_close(fo);

return 0;
}
