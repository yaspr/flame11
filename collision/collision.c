#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "flame.h"

#include "pi.h"

#define MIN_RADIUS  2
#define MAX_RADIUS 10

#define MIN_X    20
#define MAX_X 1900 //1920

#define MIN_Y   20
#define MAX_Y 1000 //1080

typedef unsigned char byte;

//
typedef struct particle_s {

  double prev_px;
  double prev_py;

  double m; //Mass
  double r; //Radius
  
  double px; //Position X
  double py; //Position Y

  double vx; //Velocity X
  double vy; //Velocity Y

  double ax; //Acceleration X
  double ay; //Acceleration Y

  byte red;
  byte green;
  byte blue;
} particle_t;
  
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
static inline void normalize(double *x, double *y)
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
  normalize(x, y);
  scale(x, y, s);
}

//
static inline double dotprod(double x1, double y1, double x2, double y2)
{  return (x1 * x2) + (y1 * y2); }

//
static inline double norm(double x, double y)
{
  return sqrt(x * x + y * y);
}

//
void draw_frame(flame_obj_t *fo, int x, int y, int w, int h)
{
  flame_draw_line(fo, x, y, x + w, y);
  flame_draw_line(fo, x, y, x, y + h);
  flame_draw_line(fo, x + w, y, x + w, y + h);
  flame_draw_line(fo, x, y + h, x + w, y + h);
}

//
void particle_draw(flame_obj_t *fo, particle_t *p)
{
  //Remove previous
  flame_set_color(fo, 0, 0, 0);
  for (double a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_line(fo, MIN_X + p->prev_px, MIN_Y + p->prev_py, MIN_X + p->prev_px + p->r * cos(a), MIN_Y + p->prev_py + p->r * sin(a));
  
  //Draw current
  flame_set_color(fo, p->red, p->green, p->blue);
  /* flame_set_color(fo, 255, 255, 255); */
  for (double a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_line(fo, MIN_X + p->px, MIN_Y + p->py, MIN_X + p->px + p->r * cos(a), MIN_Y + p->py + p->r * sin(a));
}


//Check bounds
void particle_update(particle_t *p)
{
  /* //Gravity effect F = m . a - Check if bottom reached (approx with rounding) */
  /* if (fabs(round(p->prev_py) - round(p->py)) <= 2 && round(p->py) >= (MAX_Y - MIN_Y) - p->r) */
  /*   {       */
  /*     p->prev_px = p->px; */
  /*     p->prev_py = p->py; */

  /*     p->vy = 0; */
  /*     p->ay = 0; */

  /*     p->px += p->vx / 2; */
  /*     p->py = (MAX_Y - MIN_Y) - p->r; */
      
  /*     if (p->px >= (MAX_X - MIN_X) - p->r || p->px <= (/\* MIN_X +  *\/p->r)) */
  /* 	p->vx *= -1; */
  /*   } */
  /* else */
  /*   { */

  p->prev_px = p->px;
  p->prev_py = p->py;
  
  p->px += p->vx;
  p->py += p->vy;
  
  if (p->px >= (MAX_X - MIN_X) - p->r || p->px <= (/* MIN_X +  */p->r))
    p->vx *= -1;
  
  if (p->py >= (MAX_Y - MIN_Y) - p->r || p->py <= (/* MIN_Y +  */p->r))
    p->vy *= -1;
  
  p->vx += p->m * p->ax;
  p->vy += p->m * p->ay;

  /*   p->ay += 0.00001; */
  /* } */
}
  

//Suppose mass == radius
void particle_collision(particle_t *pl, particle_t *p, int n)
{  
  double d;

  for (int i = 0; i < n; i++)
    if (p != (pl + i) && (d = dist(p->px, p->py, pl[i].px, pl[i].py)) <= (p->r + pl[i].r))
      {
	/* //If two particles overlap, correct by half the overlaping distance */
	/* //in both directions of the distance vector  */
	double overlap = 0.5 * (d - p->r - pl[i].r);
	
	p->px -= overlap * (p->px - pl[i].px) / d;
	p->py -= overlap * (p->py - pl[i].py) / d;

	pl[i].px += overlap * (p->px - pl[i].px) / d;
	pl[i].py += overlap * (p->py - pl[i].py) / d;
	
	//Elastic collision - assume mass = radius
	double m    = (p->m + pl[i].m);
	double diff = (p->m - pl[i].m);

	double px = p->vx, py = p->vy;
	
    	p->vx = (p->vx * diff + (2 * pl[i].m * pl[i].vx)) / m;
	p->vy = (p->vy * diff + (2 * pl[i].m * pl[i].vy)) / m;
	
	pl[i].vx = ((pl[i].vx * (-diff) + (2 * p->m * px)) / m);
	pl[i].vy = ((pl[i].vy * (-diff) + (2 * p->m * py)) / m);

       	//Swapping colors (example: heat exchange)
	byte red = pl[i].red, green = pl[i].green, blue = pl[i].blue;
	
	pl[i].red   = p->red;
	pl[i].green = p->green;
	pl[i].blue  = p->blue;
	
	p->red = red;
	p->green = green;
	p->blue = blue;
      }
}

//
void particle_init(particle_t *pl, int n)
{
  int inters;
  
  pl[0].px = randxy(MIN_X + 10, MAX_X - MIN_X - 10);
  pl[0].py = randxy(MIN_Y + 10, MAX_Y - MIN_Y - 10);

  pl[0].m = randxy(5, 30);
  pl[0].r = pl[0].m;

  int s1 = (randxy(0, 1)) ? -1 : 1;
  int s2 = (randxy(0, 1)) ? -1 : 1;
  
  pl[0].vx = s1 * randxy(1, 5);
  pl[0].vy = s2 * randxy(1, 5);

  /* pl[0].ax = 0; */
  /* pl[0].ay = 0.01; */

  pl[0].red   = randxy(0, 255);
  pl[0].green = randxy(0, 255);
  pl[0].blue  = randxy(0, 255);
  
  for (int i = 1; i < n; )
    {
      double px = randxy(MIN_X + 10, MAX_X - MIN_X - 10);
      double py = randxy(MIN_Y + 10, MAX_Y - MIN_Y - 10);
      double  r = randxy(5, 30);
      
      inters = 0;
      
      //Check if new circle position is not overlapping another circle 
      for (int j = 0; !inters && j < i; j++)
	inters = (round(dist(px, py, pl[j].px, pl[j].py)) <= round(r + pl[j].r) + 2);
      
      if (!inters)
	{
	  pl[i].px = px;
	  pl[i].py = py;
	  
	  pl[i].r = r;
	  pl[i].m = pl[i].r;
	  
	  s1 = (randxy(0, 1)) ? -1 : 1;
	  s2 = (randxy(0, 1)) ? -1 : 1;
	  
	  pl[i].vx = s1 * randxy(1, 5);
	  pl[i].vy = s2 * randxy(1, 5);

	  /* pl[i].ax = 0; */
	  /* pl[i].ay = 0.01; */

	  pl[i].red   = randxy(0, 255);
	  pl[i].green = randxy(0, 255);
	  pl[i].blue  = randxy(0, 255);
	  
	  i++;
	}
    }
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int prev_click_x, prev_click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1080; 
  flame_obj_t *fo = flame_open("Collision", x_max, y_max);

  XEvent event;
  
  srand(getpid());
  
  int nb_particles = 100;
  particle_t pl[nb_particles];

 run:
  
  particle_init(pl, nb_particles);
  
  flame_clear_display(fo);
  
  //
  while (1)
    {
      draw_frame(fo, MIN_X, MIN_Y, MAX_X - MIN_X, MAX_Y - MIN_Y);
      
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
	    {
	      if (event.type == ButtonPress)
		{
		}
	    }
	}
      else
	{
	  for (int i = 0; i < nb_particles; i++)
	    {
	      particle_collision(pl, &pl[i], nb_particles);
	      particle_update(&pl[i]);
	      particle_draw(fo, &pl[i]);
	    }
	  
	  usleep(20000);
	}
    }

  flame_close(fo);

  return 0;
}
