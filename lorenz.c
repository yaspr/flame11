#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include "flame.h"

#include "rdtsc.h"

//
void delay(unsigned millis) 
{ 
  // Converting time into milli_seconds 
  int milli_seconds = millis; 
  
  // Stroing start time 
  clock_t start_time = clock(); 
  
  // looping till required time is not acheived 
  while (clock() < start_time + milli_seconds) 
    ; 
}

//Lorenz attractor
void lorenz(flame_obj_t *fo, FILE *fd, unsigned iter)
{
  double xt, yt, zt;
  double x = 0.1, y = 0.0, z = 0.0;
  const double a = 10.0, b = 50.0, c = 8.0 / 3.0, t = 0.01; 
  
  for (unsigned i = 0; i < iter; i++)
    {
      xt = x + t * a * (y - x);
      yt = y + t * (x * (b - z) - y);
      zt = z + t * (x * y - c * z);

      x = xt;
      y = yt;
      z = zt;

      //Writing coordinates to a file
      //Using gnuplot to plot in a 3D space: gnuplot> splot OUTPUT_FILE
      fprintf(fd, "%lf %lf %lf\n", x, y, z);
      
      //flame_draw_point(fo, x + 200, y + 200);
      flame_draw_point(fo, y + 200, z + 200);
    }
}

//
int main(int argc, char **argv)
{
  if (argc < 2)
    return printf("OUPS! %s [output file]\n", argv[0]), -1;
  
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  FILE *fd = fopen(argv[1], "wb");
  flame_obj_t *fo = flame_open("Bezier curve", x_max, y_max);

  if (!fd)
    return printf("OUPS! Cannot create file %s. Check permissions or available disk size.\n", argv[1]), -2;
  
  flame_wait(fo, &click_x, &click_y);
  
 run:
  
  lorenz(fo, fd, 1000);
  
  //printf("Done\n");
  
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
	    goto run; //No comment!
    }

  fclose(fd);
  flame_close(fo);
  
  return 0;
}
