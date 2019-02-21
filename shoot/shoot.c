#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>

#include "pi.h"

#include "flame.h"

#define min(a, b) ((a) > (b)) ? (b) : (a)
#define max(a, b) ((a) > (b)) ? (a) : (b)

#define MAX_X 1920
#define MAX_Y 1080

#define MAX_STARS 1000

//
typedef unsigned char byte;

//
typedef struct xy_s { double x; double y; } xy_t;

//
typedef struct ppm_s {

  int w; //Width
  int h; //Height 
  int t; //Threshold
  byte *px; } ppm_t;

//
typedef struct vehicle_s {

  double prev_p1_x;
  double prev_p1_y;

  double prev_p2_x;
  double prev_p2_y;

  double prev_p3_x;
  double prev_p3_y;

  double prev_cx;
  double prev_cy;
  
  double angle;

  double m;

  double r;
  
  double p1_x;
  double p1_y;

  double p2_x;
  double p2_y;

  double p3_x;
  double p3_y;

  double cx;
  double cy;

  double prev_vx;
  double prev_vy;
  
  double vx;
  double vy;

  double ax;
  double ay;

  double max_velocity;
  double max_acceleration;

  ppm_t *image;
  
} vehicle_t;
  
//
typedef struct particle_s {
  
  double prev_px;
  double prev_py;

  double r; //Particle radius

  double m; //Particle mass

  //Position
  double px;
  double py;

  //Velocity
  double vx;
  double vy;

  //Acceleration
  double ax;
  double ay;

  double max_velocity;

  int shot_dead;
  int shot_len;

  ppm_t *image;

} particle_t;

//
static inline int randxy(int a, int b)
{ return (rand() % (b - a + 1)) + a; }

//
static inline double dist(double x1, double y1, double x2, double y2)
{
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
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
ppm_t *ppm_open(char *fname)
{
  char c0, c1, c;
  FILE *fd = fopen(fname, "rb");

  if (fd)
    {
      ppm_t *p = malloc(sizeof(ppm_t));
      
      fscanf(fd, "%c%c\n", &c0, &c1);

      c = fgetc(fd);

      if (c == '#')
	{
	  //Handle comment
	  while (c != '\n')
	    c = fgetc(fd);
	}
      else
	fseek(fd, -1, SEEK_CUR);
      
      fscanf(fd, "%d %d\n", &p->w, &p->h);
      fscanf(fd, "%d\n", &p->t);
            
      p->px = malloc(sizeof(byte) * p->w * p->h * 3);
      
      if (c0 == 'P')
	if (c1 == '6') //Binary mode
	  {
	    fread(p->px, sizeof(byte), p->w * p->h * 3, fd);
	  }
	else
	  if (c1 == '3') //ASCII mode
	    {
	    }
      
      fclose(fd);

      return p;
    }
  else
    return NULL;
}

//Pick one pixel out of 2 & 1 row out of two
void ppm_zoom_out_x2(ppm_t *p)
{
  int l = 0, m = 0;
  //Half the width, half the hight, 3 bytes per pixel 
  byte *new_px = malloc(sizeof(byte) * (p->w >> 1) * (p->h >> 1) * 3);

  //Pointer hacking :) (1D ==> 2D) 
  byte (*p_px)[p->w * 3] = (byte (*)[p->w * 3]) p->px;
  byte (*p_new_px)[(p->w >> 1) * 3] = (byte (*)[(p->w >> 1) * 3]) new_px;
  
  for (int i = 0; i < p->h && l < (p->h >> 1); i += 2, l++)
    {
      m = 0;
      
      for (int j = 0, k = 0; j < p->w; j += 2, k += 6, m += 3)
	{
	  byte r = p_px[i][k];
	  byte g = p_px[i][k + 1];
	  byte b = p_px[i][k + 2];

	  p_new_px[l][m]     = r;
	  p_new_px[l][m + 1] = g;
	  p_new_px[l][m + 2] = b;
	}
    }
  
  free(p->px);
  
  p->w  = p->w >> 1;
  p->h  = p->h >> 1;
  p->px = new_px;
}

//Draws only active pixels 
void ppm_draw(flame_obj_t *fo, double x, double y, ppm_t *p)
{
  int k = 0;
  
  for (int h = 0; h < p->h; h++)
    for (int w = 0; w < p->w; w++, k += 3)
      {
	if (p->px[k] || p->px[k + 1] || p->px[k + 2])
	  {
	    flame_set_color(fo, p->px[k], p->px[k + 1], p->px[k + 2]);
	    flame_draw_point(fo, x + w, y + h);
	  }
      }
}

//Clears active pixels
void ppm_clear(flame_obj_t *fo, double x, double y, ppm_t *p)
{
  int k = 0;
  
  for (int h = 0; h < p->h; h++)
    for (int w = 0; w < p->w; w++, k += 3)
      {
	if (p->px[k] || p->px[k + 1] || p->px[k + 2])
	  {
	    flame_set_color(fo, 0, 0, 0);
	    flame_draw_point(fo, x + w, y + h);
	  }
      }
}

//
static inline int particle_collide(particle_t *p1, particle_t *p2)
{
  return (dist(p1->px, p1->py, p2->px, p2->py) <= (p1->r + p2->r));
}

//
void particle_show(flame_obj_t *fo, particle_t *p)
{
  flame_set_color(fo, 0, 0, 0);
  //flame_draw_point(fo, p->prev_px, p->prev_py);
  for (double a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_point(fo, p->prev_px + p->r * cos(a), p->prev_py + p->r * sin(a));

  flame_set_color(fo, 255, 255, 255);
  //flame_draw_point(fo, p->px, p->py);
  for (double a = 0.0; a < 2 * PI; a += 0.01)
    flame_draw_point(fo, p->px + p->r * cos(a), p->py + p->r * sin(a));
}

//
void particle_draw(flame_obj_t *fo, particle_t *p)
{
  //Draw ppm vehicle (preferably B&W)
  //Remove previous
  ppm_clear(fo, p->prev_px - p->image->w / 2, p->prev_py - p->image->h / 2, p->image);

  //Draw current
  ppm_draw(fo, p->px - p->image->w / 2, p->py - p->image->h / 2, p->image);
}

//
void particle_update(particle_t *p)
{
  p->prev_px = p->px;
  p->prev_py = p->py;

  p->px += p->vx;
  p->py += p->vy;
  
  p->vx += p->m * p->ax;
  p->vy += p->m * p->ay;

  if (p->px <= p->r || p->px >= MAX_X - p->r)
    p->vx *= -1;

  if (p->py <= p->r || p->py >= MAX_Y - p->r)
    p->vy *= -1;
  
  limit(&p->vx, &p->vy, p->max_velocity);

  p->ax = p->ay = 0;
}

//Returns 1 if OOB
int vehicle_check_bounds(vehicle_t *v)
{
  return
    (v->p1_x <= 0.0 || v->p1_x >= MAX_X) ||
    (v->p1_y <= 0.0 || v->p1_y >= MAX_Y) ||
    (v->p2_x <= 0.0 || v->p2_x >= MAX_X) ||
    (v->p2_y <= 0.0 || v->p2_y >= MAX_Y) ||
    (v->p3_x <= 0.0 || v->p3_x >= MAX_X) ||
    (v->p3_y <= 0.0 || v->p3_y >= MAX_Y) ;
}

//
void vehicle_update(vehicle_t *v)
{
  //
  v->prev_p1_x = v->p1_x;
  v->prev_p1_y = v->p1_y;

  v->prev_p2_x = v->p2_x;
  v->prev_p2_y = v->p2_y;

  v->prev_p3_x = v->p3_x;
  v->prev_p3_y = v->p3_y;

  v->prev_cx = v->cx;
  v->prev_cy = v->cy;
  
  //
  v->p1_x += v->vx;
  v->p1_y += v->vy;

  v->p2_x += v->vx;
  v->p2_y += v->vy;
  
  v->p3_x += v->vx;
  v->p3_y += v->vy;

  v->angle = atan(v->vy / v->vx);
  
  v->cx = (v->p1_x + v->p2_x + v->p3_x) / 3.0;
  v->cy = (v->p1_y + v->p2_y + v->p3_y) / 3.0;
  
  v->prev_vx = v->vx;
  v->prev_vy = v->vy;
  
  v->vx += v->m * v->ax;
  v->vy += v->m * v->ay;
  
  limit(&v->vx, &v->vy, v->max_velocity);
  limit(&v->ax, &v->ay, v->max_acceleration);
}

//
void vehicle_show(flame_obj_t *fo, vehicle_t *v)
{
  //Draw vehicle as triangle
  //Remove previous triangle - draw black (can cause artifacts)
  flame_set_color(fo, 0, 0, 0);
  flame_draw_line(fo, v->prev_cx, v->prev_cy, v->prev_cx + 10 * v->prev_vx, v->prev_cy + 10 * v->prev_vy);
  //
  flame_draw_line(fo, v->prev_p1_x, v->prev_p1_y, v->prev_p2_x, v->prev_p2_y);
  flame_draw_line(fo, v->prev_p1_x, v->prev_p1_y, v->prev_p3_x, v->prev_p3_y);
  flame_draw_line(fo, v->prev_p3_x, v->prev_p3_y, v->prev_p2_x, v->prev_p2_y);
  
  //Draw current
  //Velocity vector
  flame_set_color(fo, 0, 0, 255);
  flame_draw_line(fo, v->cx, v->cy, v->cx + 10 * v->vx, v->cy + 10 * v->vy);
  //Vehicle
  flame_set_color(fo, 255, 255, 255);
  flame_draw_line(fo, v->p1_x, v->p1_y, v->p2_x, v->p2_y);
  flame_draw_line(fo, v->p1_x, v->p1_y, v->p3_x, v->p3_y);
  flame_draw_line(fo, v->p3_x, v->p3_y, v->p2_x, v->p2_y);
}

//
void vehicle_draw(flame_obj_t *fo, vehicle_t *v)
{
  double scale = 20;
  
  //Draw ppm vehicle (preferably B&W)
  //Remove previous
  flame_set_color(fo, 0, 0, 0);
  flame_draw_line(fo, v->prev_cx, v->prev_cy, v->prev_cx + scale * v->prev_vx, v->prev_cy + scale * v->prev_vy);
  ppm_clear(fo, v->prev_cx - v->image->w / 2, v->prev_cy - v->image->h / 2, v->image);

  //Draw current
  ppm_draw(fo, v->cx - v->image->w / 2, v->cy - v->image->h / 2, v->image);

  //Velocity vector on top of image
  flame_set_color(fo, 0, 0, 255);
  flame_draw_line(fo, v->cx, v->cy, v->cx + scale * v->vx, v->cy + scale * v->vy);
}

//
void draw_stars(flame_obj_t *fo, double *xy, int n)
{
  for (int i = 0; i < n; i += 2)
    {
      flame_set_color(fo, 255, 255, 255);
      flame_draw_point(fo, xy[i], xy[i + 1]);
    }
}


//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  flame_obj_t *fo = flame_open("", x_max, y_max);
  
  XEvent event;

  srand(getpid());

  int nb_stars = 100;
  double xy[MAX_STARS];
  
  double dt = 0;
  double elapsed = 0;
  double time_step_s = 0;
    
  vehicle_t v1;
  int shot_len = 0;
  particle_t p1, p2;
  int show_attacker = 0;
  int nb_particles = 10;
  particle_t pl[nb_particles];

  int SHOT_SCOPE = 20;
  int ATTACK_DELAY = 5;

  ppm_t *images[3];
  
  images[0] = ppm_open("ppm/asteroid1.ppm");
  images[1] = ppm_open("ppm/asteroid2.ppm");
  images[2] = ppm_open("ppm/asteroid3.ppm");
  
  ppm_zoom_out_x2(images[0]);
  ppm_zoom_out_x2(images[0]);
 
  ppm_zoom_out_x2(images[1]);
  ppm_zoom_out_x2(images[1]);
  
  ppm_zoom_out_x2(images[2]);
  ppm_zoom_out_x2(images[2]);
    
  struct timeval before, after;
  unsigned long long frame_count = 0;
  
  v1.image = ppm_open("ppm/ufo.ppm");
  ppm_zoom_out_x2(v1.image);
  
  p2.image = ppm_open("ppm/attacker2.ppm");
  ppm_zoom_out_x2(p2.image);
  ppm_zoom_out_x2(p2.image);
  
 run:

  SHOT_SCOPE = 20;
  ATTACK_DELAY = 5;
  
  show_attacker = 0;

  shot_len = 0;
  nb_particles = 10;
  
  p1.max_velocity = 0;

  time_step_s = 0;
  
  v1.m = 40;
  
  //
  v1.p1_x = randxy(10, 1000);
  v1.p1_y = v1.p1_x;

  v1.p2_x = v1.p1_x + v1.m;
  v1.p2_y = v1.p1_y;

  v1.p3_x = (v1.p1_x + v1.p2_x) / 2;
  v1.p3_y = (v1.p1_x + v1.p2_x) / 2;

  v1.cx = (v1.p1_x + v1.p2_x + v1.p3_x) / 3.0;
  v1.cy = (v1.p1_y + v1.p2_y + v1.p3_y) / 3.0;
  
  v1.r = dist(v1.p1_x, v1.p1_y, v1.cx, v1.cy);
  
  v1.vx = 0;
  v1.vy = 0;
  
  v1.ax = 0.01;
  v1.ay = 0.01;

  v1.max_velocity = 1;
  v1.max_acceleration = 2;

  for (int i = 0; i < nb_stars; i += 2)
    {
      xy[i]     = randxy(10, 1900);
      xy[i + 1] = randxy(10, 1000); 
    }
  
  //
  for (int i = 0; i < nb_particles; i++)
    {
      //
      pl[i].px = randxy(100, 1000);
      pl[i].py = randxy(100, 1000);
      
      pl[i].r = 25;
      pl[i].m = pl[i].r;

      pl[i].vx = 0;
      pl[i].vy = 0;

      int s = (randxy(0, 1)) ? -1 : 1;
      
      pl[i].ax = s * 0.0001;
      pl[i].ay = s * 0.0001;

      pl[i].max_velocity = 0.5;

      pl[i].shot_dead = 0;

      int i_id = randxy(0, 2);

      pl[i].image = images[i_id];

    }
        
  flame_clear_display(fo);
  flame_set_color(fo, 255, 255, 255);

  gettimeofday(&before, NULL);

  //Problem: maintain a constant FPS even with lesser objects to draw 
  while (1)
    {      
      gettimeofday(&after, NULL);
      
      elapsed = fabs(after.tv_sec - before.tv_sec);
      
      frame_count++;

      before = after;

      draw_stars(fo, xy, nb_stars);
      
      if (XPending(fo->display) > 0)
	{
	  XNextEvent(fo->display, &event);
	  
	  if (event.type == KeyPress)
	    {
	      c = XLookupKeysym(&event.xkey, 0);

	      if (c == 'q')
		break;
	      else
		if (c == 82) //111
		  {
		    v1.ay -= 0.5;
		  }
		else
		  if (c == 84) //116 
		    {
		      v1.ay += 0.5;
		    }
		  else
		    if (c == 83) //114
		      {
			v1.ax += 0.5;
		      }
		    else
		      if (c == 81) //113
			{
			  v1.ax -= 0.5;
			}
		      else
			if (c == 's')
			  {
			    v1.max_velocity *= 0.8;
			  }
			else
			  if (c == 'a')
			    {
			      v1.max_velocity *= 1.2;
			    }
			  else
			    if (c == ' ')
			      {				
				double d = dist(v1.cx, v1.vy, v1.vx, v1.vy);
				
				p1.px = v1.cx + (v1.vx) / d;
				p1.py = v1.cy + (v1.cy) / d;
				
				p1.r = 2;
				p1.m = p1.r;
				
				p1.vx = 10 * (v1.vx); /// d;
				p1.vy = 10 * (v1.vy); /// d;
				
				p1.ax = 0.1;
				p1.ay = 0.1;

				p1.max_velocity = 10;

				shot_len = 0;
			      }
	    }
	  else
	    if (event.type == ButtonPress)
	      {
	      }
	}
      else
	{
	  //Bounce on bounds --> Dead
	  if (!vehicle_check_bounds(&v1))
	    vehicle_draw(fo, &v1);
	  else
	    {
	      goto run;
	    }	  

	  vehicle_update(&v1);
	  
	  //Bounce into an asteroid --> Dead
	  for (int i = 0; i < nb_particles; i++)
	    {
	      if (dist(v1.cx, v1.cy, pl[i].px, pl[i].py) <= (v1.r + pl[i].r))
		{
		  goto run;
		}

	      //Bounce into alien attacker --> Dead
	      if (show_attacker && dist(v1.cx, v1.cy, p2.px, p2.py) <= (v1.r + p2.r))
		{
		  goto run;
		}
	    }
	  
	  //
	  for (int i = 0; i < nb_particles; i++)
	    if (!pl[i].shot_dead)
	      {
		particle_draw(fo, &pl[i]);
		particle_update(&pl[i]);
	      }
	  
	  //Simulate a shot - Distance of SHOT_SCOPE
	  if (shot_len < SHOT_SCOPE)
	    {
	      //Check is attacker is hit
	      if (show_attacker)
		{
		  if (particle_collide(&p1, &p2))
		    {
		      /* printf("Dead attacker %d\n", ATTACK_DELAY); */
		      
		      //Erase bullet
		      flame_set_color(fo, 0, 0, 0);
		      for (double a = 0.0; a < 2 * PI; a += 0.01)
			flame_draw_point(fo, p1.prev_px + p1.r * cos(a), p1.prev_py + p1.r * sin(a));

		      //Erase attacker
		      ppm_clear(fo, p2.prev_px - p2.image->w / 2, p2.prev_py - p2.image->h / 2, p2.image);
		      
		      p1.max_velocity = 0;
		      p2.max_velocity = 0;
		      
		      show_attacker = 0;

		      SHOT_SCOPE += 30;
		      
		      ATTACK_DELAY += 5;
		    }
		}

	      //If a bullet hits a ball
	      for (int i = 0; i < nb_particles; i++)
		if (particle_collide(&p1, &pl[i]))
		  {
		    pl[i].shot_dead = 1;
		    
		    //Erase bullet
		    flame_set_color(fo, 0, 0, 0);
		    for (double a = 0.0; a < 2 * PI; a += 0.01)
		      flame_draw_point(fo, p1.prev_px + p1.r * cos(a), p1.prev_py + p1.r * sin(a));

		    //Erase shot object
		    ppm_clear(fo, pl[i].prev_px - pl[i].image->w / 2, pl[i].prev_py - pl[i].image->h / 2, pl[i].image);
		    
		    //Remove hit particle  
		    pl[i] = pl[nb_particles - 1];
		    nb_particles--;
		    
		    p1.max_velocity = 0;
		    
		    //Gain more scope
		    SHOT_SCOPE += 20;
		  }

	      if (nb_particles == 0)
		{
		  goto run;
		}
	      
	      //Keep going otherwise
	      particle_show(fo, &p1);
	      particle_update(&p1);
	      shot_len++;
	    }
	  else
	    {
	      if (shot_len >= SHOT_SCOPE)
		{
		  shot_len = 0;
		  p1.max_velocity = 0;
		  
		  //Erase particle
		  flame_set_color(fo, 0, 0, 0);
		  for (double a = 0.0; a < 2 * PI; a += 0.01)
		    flame_draw_point(fo, p1.prev_px + p1.r * cos(a), p1.prev_py + p1.r * sin(a));		  
		}
	    }

	  if (show_attacker)
	    {
	      particle_draw(fo, &p2);
	      particle_update(&p2);
	    }
	}

      //1 second 
      if (elapsed == 1)
	{
	  printf("FPS: %llu\n", frame_count);
	  frame_count = 0;
	  time_step_s++;
	}
      
      //Every ATTACK_DELAY seconds
      if (time_step_s == ATTACK_DELAY)
	{
	  time_step_s = 0;
	  
	  if (nb_stars < MAX_STARS)
	    {
	      xy[nb_stars]     = randxy(10, 1900);
	      xy[nb_stars + 1] = randxy(10, 1900);
	     
	      nb_stars += 2;
	    }

	  if (!show_attacker)
	    {
	      /* printf("Showing attacker %d\n", ATTACK_DELAY); */
	      
	      //Show attacker
	      show_attacker = 1;

	      p2.px = randxy(10, 1000);
	      p2.py = randxy(10, 1000);
	  
	      p2.r = 30;
	      p2.m = p2.r;

	      p2.vx = 0;
	      p2.vy = 0;

	      p2.ax = 0.001;
	      p2.ay = 0.001;

	      p2.max_velocity = 3;
	    }
	  else
	    if (show_attacker)
	      {
		/* printf("Clearing attacker %d\n", ATTACK_DELAY); */
		
		show_attacker = 0;
		
		p2.max_velocity = 0;
		
		//Erase attacker
		ppm_clear(fo, p2.prev_px - p2.image->w / 2, p2.prev_py - p2.image->h / 2, p2.image);
	      }
	}
    }
  
  flame_close(fo);

  return 0;
}
