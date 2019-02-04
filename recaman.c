#include <math.h> 
#include <stdio.h>
#include <stdlib.h>

#include "pi.h"

#include "flame.h"

#define ANGLE_STEP 0.001

#define MAX_BUFFER 4096

//
typedef unsigned char byte;
typedef struct xy_s { double x; double y; } xy_t;
typedef struct rgb_s { byte r; byte g; byte b; } rgb_t;

//
int lookup(int v, int *seq, int n)
{
  byte found = 0;
  
  for (int i = 0; !found && i < n; i++)
    found = (seq[i] == v);

  return found;
}

//
void draw_circle(flame_obj_t *fo, xy_t base, xy_t c, double r, rgb_t color, int s)
{
  flame_set_color(fo, color.r, color.g, color.b);
  
  for (double a = 0.0 + s * PI ; a < PI + s * PI; a += ANGLE_STEP)
    flame_draw_point(fo, base.x + c.x + r * cos(a), base.y + c.y + r * sin(a));
  
  flame_set_color(fo, 255, 255, 255);
}

//
void recaman(flame_obj_t *fo, int n)
{
  double radius = 0.0;
  int seq[MAX_BUFFER];
  int scale = 5, sign; 
  xy_t base   = { 0, 300 };
  xy_t center = { 0, 300 };
  rgb_t color = { 255, 255, 255 };

  seq[0] = 0;
  
  //
  for (int i = 1; i < n; i++)
    {
      printf("%d %d\n", i, seq[i - 1]);
      
      if (seq[i - 1] - i > 0)
	{
	  if (!lookup(seq[i - 1] - i, seq, i))
	    {
	      seq[i] = seq[i - 1] - i;
	      sign = 0;
	    }
	  else
	    {
	      seq[i] = seq[i - 1] + i;
	      sign = 1;
	    }
	}
      else
	{
	  seq[i] = seq[i - 1] + i;
	  sign = 1;
	}

      radius   = scale * (i >> 1);
      center.x = scale * seq[i] + radius;
      
      draw_circle(fo, base, center, radius, color, sign);
      draw_circle(fo, base, center, radius, color, !sign);
    }
}

//
int main(int argc, char **argv)
{
  char c;
  int y_max = 1080;
  int x_max = 1920;
  int click_x = 0, click_y = 0;
  flame_obj_t *fo = flame_open("Circle", x_max, y_max);
  
  xy_t base   = { x_max / 2, y_max / 2 };
  rgb_t color = { 128, 0, 0 };
  xy_t center = { 10, 10 };
  double radius = 100;
  
 run:
  
  flame_clear_display(fo);
  
  recaman(fo, 120);
  
  while (1)
    {
      c = flame_wait(fo, &click_x, &click_y);

      if (c == 1)
	{
	  printf("%d, %d\n", click_x, click_y); break;

	  goto run;
	}
      else
	if (c == 'r')
	  goto run;
	else
	  if (c == 'q')
	    break;
    }
  
  flame_close(fo);

  return 0;
}
