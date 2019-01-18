#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "flame.h"

//New origin of the plane
#define BASE_X 600
#define BASE_Y 1000

//Scale of the image
#define SCALE  60

//
void f1(double ix, double iy, double *ox, double *oy)
{
  *ox = 0;
  *oy = 1.6 * iy;
}

//
void f2(double ix, double iy, double *ox, double *oy)
{
  *ox =  0.85 * ix + 0.04 * iy;
  *oy = -0.04 * ix + 0.85 * iy + 1.6;
}

//
void f3(double ix, double iy, double *ox, double *oy)
{
  *ox =  0.20 * ix - 0.26 * iy;
  *oy =  0.23 * ix + 0.22 * iy + 1.6;
}

//
void f4(double ix, double iy, double *ox, double *oy)
{
  *ox = -0.15 * ix + 0.28 * iy;
  *oy =  0.26 * ix + 0.24 * iy + 0.44;
}

void barnsley(flame_obj_t *fo, uint64_t iter)
{
  double r = 0.0;
  double x = 0.0, y = 0.0, ox = 0.0, oy = 0.0;
  
  for (uint64_t i = 0; i < iter; i++)
    {
      r = (double)rand() / RAND_MAX;

      if (r < 0.01)
	f1(x, y, &ox, &oy);
      else
	if (r < 0.86)
	  f2(x, y, &ox, &oy);
	else
	  if (r < 0.93)
	    f3(x, y, &ox, &oy);
	  else
	    f4(x, y, &ox, &oy);

      x = ox; y = oy;

      printf("%lf %lf %lf\n", r, x, y);
      
      flame_draw_point(fo, BASE_X + (-x * SCALE), BASE_Y + (-y * SCALE));
    }
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  flame_obj_t *fo = flame_open("Barnsley", x_max, y_max);  

  srand(getpid());
  
 run:

  //
  barnsley(fo, 1000000);
  
  while (1)
    {
      c = flame_wait(fo, &click_x, &click_y);
      
      if (c == 1)
	printf("Click at: %d, %d\n", click_x, click_y);
      else
	if (c == 'q')
	  break;
	else
	  if (c == 'r')
	    {
	      goto run; //No comment!
	    }
    }
  
  flame_close(fo);
  
  return 0;
}
