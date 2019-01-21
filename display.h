#ifndef DISPLAY_H
#define DISPLAY_H

#include "flame.h"

//
typedef struct display_s {
                            double x_curr, y_curr;
                            double x_next, y_next;

                            double angle;

                            flame_obj_t *fo;

                            int distance;
  
                            double BASE_X;
                            double BASE_Y;

                            int width;
                            int height;
                         } display_t;

//
void forward(display_t *d);
void draw_2D_plane(display_t *t);
void turn(display_t *d, double theta);
void set_2D_plane(display_t *t, int x, int y);
void init_display(display_t *d, flame_obj_t *fo, int X, int Y, int dist, int w, int h, double angle);

#endif
