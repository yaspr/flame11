#include <stdio.h>
#include "flame.h"

//
void drawMandelbrot(flame_obj_t *fo, long iter, long x_max, long y_max, int color)
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
int main()
{
  char c;
  int iter  = 1000;
  int y_max = 768;
  int x_max = 1366;
  int click_x = 0, click_y = 0;
  
  flame_obj_t *fo = flame_open("Mandelbrot Set", x_max, y_max);
  
  drawMandelbrot(fo, iter, x_max, y_max, 100);
  
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
