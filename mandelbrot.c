#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "flame.h"

#include "rdtsc.h"

//
void drawMandelbrot0(flame_obj_t *fo, long iter, long x_max, long y_max, int color)
{
  long i, px, py;
  float x, y, x0, y0, xtemp;
  float div_x0 = 3.15 / x_max, div_y0 = 2.7 / y_max;
  
  for (py = 0; py < y_max; py++)
    {
      for (px = 0; px < x_max; px++)
	{
	  i = 0;
	  
	  x = y = 0;
	  x0 = (px * div_x0) - 2.07;
	  y0 = (py * div_y0) - 1.33;
	  
	  while ((x * x + y * y) < 4.0f && i++ < iter)
	    {
	      xtemp = (x * x) - (y * y) + x0;
	      y = 2 * (x * y) + y0;
	      x = xtemp;
	    }
	  
	  flame_set_color(fo, color + i, (color << 1) + i, (color << 2) + i);
	  flame_draw_point(fo, px, py);
	}
    }
}

//
void drawMandelbrot1(flame_obj_t *fo, long iter, long x_max, long y_max, int color)
{
  long i, px, py;
  float x, y, x0, y0, x2, y2, xtemp;
  float div_x0 = 3.15 / x_max, div_y0 = 2.7 / y_max;
  
  for (px = 0; px < x_max; px++)
    {
      for (py = 0; py < y_max; py++)
	{
	  i = 0;
	  
	  x = y = 0;
	  x0 = (px * div_x0) - 2.07;
	  y0 = (py * div_y0) - 1.33;
	  x2 = x * x, y2 = y * y;
	  
	  while ((x2 + y2) < 4.0f && i++ < iter)
	    {
	      //y = 2 * (x * y) + y0;
	      y *= x;
	      y += y;
	      y += y0; 
	      x = x2 - y2 + x0;
	     
	      x2 = x * x;
	      y2 = y * y;
	    }
	  
	  flame_set_color(fo, color + i, (color << 1) + i, (color << 2) + i);
	  flame_draw_point(fo, px, py);
	}
    }
}

//
int main(int argc, char **argv)
{
  if (argc < 2)
    return printf("OUPS: %s [0 | 1]\n", argv[0]), -1;
  
  char c;
  int iter  = 10000;
  int y_max = 900;
  int x_max = 1800;
  int click_x = 0, click_y = 0;
  double before = 0.0, after = 0.0;
  
  flame_obj_t *fo = flame_open("Mandelbrot Set", x_max, y_max);

  if (!strncmp(argv[1], "0", 1))
    {
      before = rdtsc();
      drawMandelbrot0(fo, iter, x_max, y_max, 100);
      after = rdtsc();
    }
  else
    if (!strncmp(argv[1], "1", 1))
      {
	before = rdtsc();
	drawMandelbrot1(fo, iter, x_max, y_max, 100);
	after = rdtsc();
      }
  
  printf("cycles: %lf\n", (double)(after - before));
  
  while (1)
    {
      c = flame_wait(fo, &click_x, &click_y);

      if (c == 1)
	{
	  printf("%d, %d\n", click_x, click_y); break;
	}
      else
	if (c == 'q')
	  break;
    }

  flame_close(fo);
  
  return 0;
}
