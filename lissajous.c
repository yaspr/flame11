#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include "flame.h"

#include "pi.h"

#define DELTA_X 20
#define DELTA_Y 15

//
typedef unsigned char byte;

//
typedef struct circle_point_s {
  
  double x; double y;           //Center coodinates
  double r;                     //Radius
  double a;                     //Rotation angle
  double s;
  double curr_x; double curr_y; //Point moving around the circle
  double next_x; double next_y;

} circle_point_t;

//
typedef struct canvas_s {
  
  circle_point_t *row;
  circle_point_t *col;
  
} canvas_t;

//
int randxy(int a, int b)
{
  return ((rand() % b) - a + 1) % b;
}

//Create Row of 5 circles
canvas_t *init_canvas(int n, double o_x, double o_y)
{
  circle_point_t *row  = malloc(n * sizeof(circle_point_t));
  circle_point_t *col  = malloc(n * sizeof(circle_point_t));
  canvas_t *c = malloc(sizeof(canvas_t));
  
  row[0].r = 10;
  row[0].x = o_x + DELTA_X;
  row[0].y = o_y;
  row[0].a = 0.0;
  row[0].s = 0.001;
  row[0].curr_x = row[0].curr_y = 0;
  row[0].next_x = row[0].next_y = 0;

  for (int i = 1; i < n; i++)
    {
      row[i].r = 10;
      row[i].x = row[i - 1].x + 2 * row[i].r + DELTA_X;
      row[i].y = o_y;
      row[i].a = 0.0;
      row[i].s = row[i - 1].s + row[i - 1].s * 0.5;
      
      row[i].curr_x = row[i].curr_y = 0;
      row[i].next_x = row[i].next_y = 0;
    }

  col[0] = row[0];

  col[0].x = row[0].y;
  col[0].y = row[0].x + DELTA_Y;
  col[0].s = 0.003;
  
  //Overlapping firsts
  for (int i = 1; i < n; i++)
    {
      col[i]   = row[i];
      col[i].x = row[i].y;
      col[i].y = row[i].x + DELTA_Y;
      col[i].s = col[i - 1].s * 1.1;
    }
  
  c->row  = row;
  c->col  = col;
  
  return c;
}

//
void draw_circle_point(flame_obj_t *fo, circle_point_t *cp, int scale)
{
  //First circle
  //Remove current pixel
  flame_set_color(fo, 0, 0, 0);
  flame_draw_point(fo, scale * cp->curr_x, scale * cp->curr_y);
  
  cp->next_x = cp->x + cp->r * cos(cp->a);
  cp->next_y = cp->y + cp->r * sin(cp->a);
  
  //Draw next pixel
  flame_set_color(fo, 255, 255, 255);
  flame_draw_point(fo, scale * cp->next_x, scale * cp->next_y);

  cp->a += cp->s;
  
  if (cp->a >= 2 * PI)
    cp->a = 0.0;
  
  cp->curr_x = cp->next_x;
  cp->curr_y = cp->next_y;
}

//
void draw_lissajous_point(flame_obj_t *fo, canvas_t *cv, int scale, int n)
{
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      flame_draw_point(fo, DELTA_X + scale * cv->row[i].next_x, scale * cv->col[j].next_y);
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  flame_obj_t *fo = flame_open("Bounce", x_max, y_max);
  
  XEvent event;

  int scale = 4;
  circle_point_t c1, c2, c3;
  double curr_x = 0, curr_y = 0, next_x = 0, next_y = 0;
  
  int nb_circles = 6;
  int c_x = 0, c_y = 0;
  canvas_t *cv = init_canvas(nb_circles, 20, 20);
  
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
		click_x = event.xkey.x;
		click_y = event.xkey.y;

		c_x = randxy(0, nb_circles);
		c_y = randxy(0, nb_circles);

		printf("%d %d\n", c_x, c_y);
		
		flame_clear_display(fo);
	      }
	}
      else
	{
	  for (int i = 0; i < nb_circles; i++)
	    {
	      draw_circle_point(fo, &cv->row[i], scale);
	      draw_circle_point(fo, &cv->col[i], scale);
	    }
	  
	  //Draw curve
	  draw_lissajous_point(fo, cv, scale, nb_circles);
	  
	  usleep(1000); 
	}
    }
}

