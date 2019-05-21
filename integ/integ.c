#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "flame.h"

//Integral from 0 to n of f

//
float f(float x)
{
  return (x) ? sin(x) / x : 1;
}

//
int main(int argc, char **argv)
{
  char c;
  int x_max = 1920, y_max = 1080;
  int click_x, click_y;
  flame_obj_t *fo = flame_open("", x_max, y_max);

  int y_max_2 = y_max >> 1;
  
  int n = 20;
  float x, y, x1, y1, delx, a, b, s;
  
 lbl1:

  flame_clear_display(fo);
  
  a = 0;
  b = 20;
  s = 0;
  
  //
  flame_set_color(fo, 255, 255, 255);
  
  //Draw plane
  flame_draw_line(fo, 0, 0, 0, y_max); 
  flame_draw_line(fo, 0, y_max_2, x_max, y_max_2); 

  //Drawing the function
  for (int i = 0; i < x_max; i++)
    {
      x = i * b / x_max;
      y = f(x);

      flame_draw_line(fo, i, (-y_max_2) * y + y_max_2, i, y_max_2);
    }

  delx = (b - a) / n;
  y = f(a);

  //
  flame_set_color(fo, 0, 128, 0);
  
  for (int i = 0; i < n;)
    {
      s += delx * y / 2;

      flame_draw_line(fo, i * x_max / n, (-y_max_2) * y + y_max_2, i * x_max / n, y_max_2);
      i++;

      x1 = a + i * delx;
      y1 = f(x1);

      s += delx * y1 / 2;
      flame_draw_line(fo, i * x_max / n, (-y_max_2) * y1 + y_max_2, i * x_max / n, y_max_2);

      //
      if (signbit(y) == signbit(y1))
	{
	  flame_draw_line(fo, (i - 1) * x_max / n, (-y_max_2) * y + y_max_2, i * x_max / n, (-y_max_2) * y1 + y_max_2);
	  flame_draw_line(fo, (i - 1) * x_max / n, y_max_2, i * x_max / n, y_max_2);
	}
      else
	{
	  flame_draw_line(fo, (i - 1) * x_max / n, (-y_max_2) * y + y_max_2, i * x_max / n, y_max_2);
	  flame_draw_line(fo, (i - 1) * x_max / n, y_max_2, i * x_max / n,  (-y_max_2) * y1 + y_max_2);
	}
      
      //
      y = y1;
    }

  printf("Iterations: %d\tApproximated surface: %lf\n", n, s);
  
  c = flame_wait(fo, &click_x, &click_y);

  if (c == 'q')
    ;
  else
    {
      if (c == 'n')
	n += 10;
      if
	(c == '-')
	n -= (n) ? 1 : 0; 
      
      goto lbl1;
    }
  
  flame_close(fo);

  return 0;
}
