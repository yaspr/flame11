#include <math.h>

#include "pi.h"
#include "display.h"

//
void init_display(display_t *d, flame_obj_t *fo, int X, int Y, int dist, int w, int h, double angle)
{
  d->x_curr = d->y_curr = 0;
  d->x_next = d->y_next = 0;

  d->angle  = angle;

  d->fo = fo;

  d->distance = dist;
  
  d->BASE_X = X;
  d->BASE_Y = Y;

  d->width  = w;
  d->height = h;

  flame_clear_display(fo);
}

//
void forward(display_t *d)
{
  //Compute next position
  d->x_next = d->x_curr + d->distance * cos(d->angle);
  d->y_next = d->y_curr + d->distance * sin(d->angle);

  //printf("forward: %lf %lf, %lf %lf, %lf\n", d->x_curr, d->y_curr, d->x_next, d->y_next, d->angle);
    
  flame_draw_line(d->fo, d->BASE_X + d->x_curr, d->BASE_Y + d->y_curr,
		         d->BASE_X + d->x_next, d->BASE_Y + d->y_next);

  //Move
  d->x_curr = d->x_next;
  d->y_curr = d->y_next;
}

//
void turn(display_t *d, double theta)
{
  //printf("turn: %lf, %lf, %lf\n", d->angle, theta, d->angle + theta);
  
  d->angle += theta;
}

//
void set_2D_plane(display_t *d, int x, int y)
{
  d->BASE_X = (double)x;
  d->BASE_Y = (double)y;
}

//
void draw_2D_plane(display_t *d)
{
  flame_set_color(d->fo, 128, 0, 0);
  flame_draw_line(d->fo, d->BASE_X, 0, d->BASE_X, d->height);
  flame_draw_line(d->fo, 0, d->BASE_Y, d->width, d->BASE_Y);
  flame_set_color(d->fo, 255, 255, 255);
}
    
