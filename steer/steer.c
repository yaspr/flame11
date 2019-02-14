/*
  Craig Reynold's steering algorithm
  
*/

#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "flame.h"

#include "pi.h"

#define UFO_SIZE 10

#define MAX_POINTS 100

#define MAX_DISTANCE 900

//
typedef unsigned char byte;

//
typedef struct xy_s { double x; double y; } xy_t;

//
typedef struct vehicle_s {

  //Vehicle shape (Triangle, p1, p2, p3)
  double p1_x;
  double p1_y;
  double p2_x;
  double p2_y;
  double p3_x;
  double p3_y;

  //Centroid of the triangle
  double c_x; //(p1_x + p2_x + p3_x) / 3
  double c_y; //(p1_y + p2_y + p3_y) / 3


  //Previous position
  double prev_p1_x;
  double prev_p1_y;
  double prev_p2_x;
  double prev_p2_y;
  double prev_p3_x;
  double prev_p3_y;

  //Previous centroid
  double prev_c_x; //(p1_x + p2_x + p3_x) / 3
  double prev_c_y; //(p1_y + p2_y + p3_y) / 3

  //Acceleration
  double acc_x;
  double acc_y;

  //Velocity
  double vel_x;
  double vel_y;

  double max_speed;
  double max_force;
} vehicle_t;

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
static inline void rotate(double *x, double *y, double angle)
{
  double xx = *x, yy = *y;

  *x = cos(angle * xx) - sin(angle * yy);
  *y = cos(angle * yy) + sin(angle * xx);
}

//
void update_vehicle(vehicle_t *v)
{
  v->vel_x += v->acc_x;
  v->vel_y += v->acc_y;
  
  limit(&v->vel_x, &v->vel_y, v->max_speed);
  
  v->prev_c_x = v->c_x;
  v->prev_c_y = v->c_y;

  v->prev_p1_x = v->p1_x;
  v->prev_p1_y = v->p1_y;

  v->prev_p2_x = v->p2_x;
  v->prev_p2_y = v->p2_y;
  
  v->prev_p3_x = v->p3_x;
  v->prev_p3_y = v->p3_y;
  
  v->p1_x += v->vel_x;
  v->p1_y += v->vel_y;
  
  v->p2_x += v->vel_x;
  v->p2_y += v->vel_y;

  v->p3_x += v->vel_x;
  v->p3_y += v->vel_y;

  v->c_x = (v->p1_x + v->p2_x + v->p3_x) / 3;
  v->c_y = (v->p1_y + v->p2_y + v->p3_y) / 3;
  
  v->acc_x = 0;
  v->acc_y = 0;
}

//
void apply_force_vehicle(vehicle_t *v, double f_x, double f_y)
{
  v->acc_x += f_x;
  v->acc_y += f_y;
}

//Steer towards t (target)
void seek_vehicle(vehicle_t *v, double t_x, double t_y)
{
  double des_x = t_x - v->c_x;
  double des_y = t_y - v->c_y;

  limit(&des_x, &des_y, v->max_speed);

  double steer_x = des_x - v->vel_x;
  double steer_y = des_y - v->vel_y;

  limit(&steer_x, &steer_y, v->max_force);

  apply_force_vehicle(v, steer_x, steer_y);
}

//
void init_vehicle(vehicle_t *v)
{
  v->prev_p1_x = 0; v->prev_p1_y = 0;
  v->prev_p2_x = 0; v->prev_p2_y = 0;
  v->prev_p3_x = 0; v->prev_p3_y = 0;
  
  v->prev_c_x = 0; 
  v->prev_c_y = 0;

  int off = randxy(1, 800);
  
  v->p1_x = off +  0; v->p1_y = off +  0;
  v->p2_x = off +  0; v->p2_y = off + UFO_SIZE;
  v->p3_x = off + UFO_SIZE; v->p3_y = off +  0;

  v->c_x = UFO_SIZE / 3;
  v->c_y = UFO_SIZE / 3;

  v->acc_x = 0; v->acc_y =  0;
  v->vel_x = 0; v->vel_y = -2;

  v->max_speed = 0.9; //randxy(1, 2);
  v->max_force = 0.1;
}

//
void draw_vehicle(flame_obj_t *fo, xy_t *base, vehicle_t *v)
{
  //Remove previous triangle - draw black (can cause artifacts)
  flame_set_color(fo, 0, 0, 0);
  flame_draw_point(fo, base->x + v->prev_c_x, base->y + v->prev_c_y);
  flame_draw_line(fo, base->x  + v->prev_p1_x, base->y + v->prev_p1_y, base->x + v->prev_p2_x, base->y + v->prev_p2_y);
  flame_draw_line(fo, base->x  + v->prev_p1_x, base->y + v->prev_p1_y, base->x + v->prev_p3_x, base->y + v->prev_p3_y);
  flame_draw_line(fo, base->x  + v->prev_p3_x, base->y + v->prev_p3_y, base->x + v->prev_p2_x, base->y + v->prev_p2_y);
  
  //Draw current
  flame_set_color(fo, randxy(128, 255), randxy(128, 255), randxy(128, 255));
  flame_draw_point(fo, base->x + v->c_x, base->y + v->c_y);
  flame_draw_line(fo, base->x  + v->p1_x, base->y + v->p1_y, base->x + v->p2_x, base->y + v->p2_y);
  flame_draw_line(fo, base->x  + v->p1_x, base->y + v->p1_y, base->x + v->p3_x, base->y + v->p3_y);
  flame_draw_line(fo, base->x  + v->p3_x, base->y + v->p3_y, base->x + v->p2_x, base->y + v->p2_y);
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

  xy_t base;
  int nb_vehicles = 9;
  vehicle_t v[nb_vehicles];

  int nb_points = 0;
  double points[2 * MAX_POINTS];
  
  base.x = base.y = 0;

  //
  
  nb_points = 0;
  
  for (int i = 0; i < nb_vehicles; i++)
    init_vehicle(&v[i]);
  
  flame_clear_display(fo);
  
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

		//Store a point
		points[nb_points    ] = click_x;
		points[nb_points + 1] = click_y;

		nb_points += 2;
	      }
	}
      else
	{
	  flame_set_color(fo, 255, 255, 255);
	  for (int i = 0; i < nb_points; i += 2)
	    {
	      for (double a = 0; a < 2 * PI; a += 0.1)
		flame_draw_point(fo, points[i] + 2 * cos(a), points[i + 1] + 2 * sin(a));
	    }
	  
	  for (int i = 0; i < nb_vehicles; i++)
	    for (int j = 0; j < nb_points; j += 2)
	      {
		{
		  if (dist(v[i].c_x, v[i].c_y, points[j], points[j + 1]) < MAX_DISTANCE)
		    { 
		      seek_vehicle(&v[i], points[j], points[j + 1]);
		      update_vehicle(&v[i]);
		      draw_vehicle(fo, &base, &v[i]);
		    }
		}
	      }
		    
	  usleep(10000);
	}
    }

  flame_close(fo);

  return 0;
}
