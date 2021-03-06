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

#define A_STEP 0.01

#define MIN_RADIUS 5 
#define MAX_RADIUS 5 

#define SCOPE 10

//
typedef unsigned char byte;

//
typedef struct boid_s {

  double prev_pos_x;
  double prev_pos_y;
  
  double pos_x;
  double pos_y;
  
  double vel_x;
  double vel_y;

  //Acceleration: F = m . a (Newton's Law)
  //We suppose our objects are of mass 1
  double acc_x;
  double acc_y;

  double rad;
  
  double max_speed;

  //Boid color
  byte r;
  byte g;
  byte b;
  
} boid_t;

//
int randxy(int a, int b)
{ return (rand() % (b - a + 1)) + a; }

//
double dist_boid(boid_t *b1, boid_t *b2)
{
  double x = (b1->pos_x - b2->pos_x);
  double y = (b1->pos_y - b2->pos_y);

  return sqrt(x * x + y * y);
}

//
void fill_boid(flame_obj_t *fo, boid_t *b)
{
  flame_set_color(fo, 0, 0, 0);

  for (double angle = 0.0; angle < 2 * PI; angle += A_STEP)
    flame_draw_line(fo, b->prev_pos_x, b->prev_pos_y, b->prev_pos_x + b->rad * cos(angle), b->prev_pos_y + b->rad * sin(angle));

  flame_set_color(fo, b->r, b->g, b->b);

  for (double angle = 0.0; angle < 2 * PI; angle += A_STEP)
    flame_draw_line(fo, b->pos_x, b->pos_y, b->pos_x + b->rad * cos(angle), b->pos_y + b->rad * sin(angle));
}

//
void draw_boid(flame_obj_t *fo, boid_t *b)
{
  //Remove previous
  flame_set_color(fo, 0, 0, 0);
  for (double angle = 0.0; angle < 2 * PI; angle += A_STEP)
    flame_draw_point(fo, b->prev_pos_x + b->rad * cos(angle), b->prev_pos_y + b->rad * sin(angle)); 
  
  //Draw current
  flame_set_color(fo, 255, 255, 255);
  for (double angle = 0.0; angle < 2 * PI; angle += A_STEP)
    flame_draw_point(fo, b->pos_x + b->rad * cos(angle), b->pos_y + b->rad * sin(angle)); 
}

//
void update_boid(boid_t *b)
{
  b->prev_pos_x = b->pos_x;
  b->prev_pos_y = b->pos_y;
  
  b->pos_x += (b->pos_x > 0 && b->pos_x < 1920) ? b->vel_x : 0;
  b->pos_y += (b->pos_y > 0 && b->pos_y < 1080) ? b->vel_y : 0;
  
  b->vel_x += b->acc_x;
  b->vel_y += b->acc_y;

  double d = sqrt(b->vel_x * b->vel_x + b->vel_y * b->vel_y);

  if (d > b->max_speed)
    {
      b->vel_x /= d;
      b->vel_y /= d;
    }
  
  b->acc_x = 0;
  b->acc_y = 0;
}

//
void align_boids(boid_t *bl, boid_t *b, int n, double *x, double *y)
{
  double d = 0;
  double avg_x = 0, avg_y = 0;
  byte avg_r = 0, avg_g = 0, avg_b = 0;

  for (int i = 0; i < n; i++)
    {
      if ((bl + i) != b)
	{
	  avg_x += bl[i].vel_x;
	  avg_y += bl[i].vel_y;
	}
    }

  avg_x /= n;
  avg_y /= n;

  d = sqrt(avg_x * avg_x + avg_y * avg_y);
  
  if (d > b->max_speed)
    {
      avg_x /= d;
      avg_y /= d;
    }
  
  for (int i = 0; i < n; i++)
    {
      if ((bl + i) != b)
	{
	  b[i].vel_x -= avg_x;
	  b[i].vel_y -= avg_y;
	}
    }

  *x = avg_x;
  *y = avg_y;
}


//
void align_boid(boid_t *bl, boid_t *b, int n, double *x, double *y)
{
  int nb_neighb = 0;
  double d = 0;
  double avg_x = 0, avg_y = 0;
  byte avg_r = 0, avg_g = 0, avg_b = 0;
  
  for (int i = 0; i < n; i++)
    {
      if ((bl + i) != b)
	{
	  if (dist_boid(b, &bl[i]) < SCOPE)
	    {
	      avg_x += bl[i].vel_x;
	      avg_y += bl[i].vel_y;

	      nb_neighb++;
	    }
	}
    }
  
  if (nb_neighb)
    {
      avg_x /= nb_neighb;
      avg_y /= nb_neighb;
      
      //Sqrt & rsqrt
      d = sqrt(avg_x * avg_x + avg_y * avg_y);
      
      if (d > b->max_speed)
	{
	  avg_x /= d;
	  avg_y /= d;
       	}
      
      b->vel_x -= avg_x;
      b->vel_y -= avg_y;      
    }

  *x = avg_x;
  *y = avg_y;
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  flame_obj_t *fo = flame_open("Bounce", x_max, y_max);

  srand(getpid());
  
  XEvent event;

  double x, y;
  int nb_boids = 10;
  
  boid_t b[nb_boids * nb_boids * nb_boids], t;

 run:

  flame_clear_display(fo);

  nb_boids = 0;
  
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
	      else
		if (c == 'r')
		  goto run;
	    }
	  else
	    if (event.type == ButtonPress)
	      {
		click_x = event.xkey.x;
		click_y = event.xkey.y;
		    
		b[nb_boids].prev_pos_x = click_x;
		b[nb_boids].prev_pos_y = click_y;
		    
		b[nb_boids].pos_x = b[nb_boids].prev_pos_x;
		b[nb_boids].pos_y = b[nb_boids].prev_pos_y;
		    
		b[nb_boids].vel_x = randxy(15, 30) / 15;
		b[nb_boids].vel_y = randxy(15, 30) / 15;
		    
		b[nb_boids].acc_x = 0;
		b[nb_boids].acc_y = 0;
		    
		b[nb_boids].rad = MAX_RADIUS; //randxy(MIN_RADIUS, MAX_RADIUS);
		    
		b[nb_boids].max_speed = 10;
		    
		b[nb_boids].r = randxy(128, 255);
		b[nb_boids].g = randxy(128, 255);
		b[nb_boids].b = randxy(128, 255);
		    
		nb_boids++;
	      }
	}
      else
	{
	  //Magic happens here

	  for (int i = 0; i < nb_boids; i++)
	    {
	      align_boids(b, &b[i], nb_boids, &x, &y);

	      b[i].acc_x = x;
	      b[i].acc_y = y;

	      update_boid(&b[i]);

	      fill_boid(fo, &b[i]);
	    }
	  
	  usleep(100000); 
	}
    }

  flame_close(fo);

  return 0;
}

