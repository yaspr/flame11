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

#define STEP 0.0001

#define MAX_STALL_ITER 10000

#define MAX_CURVES 5

#define NB_POINTS 4

#define DELAY 15

#define pow2(a) ((a) * (a))
#define pow3(a) ((a) * (a) * (a))

//
unsigned randxy(unsigned x, unsigned y)
{ return (x + (rand() % y)) % y; }

//
void cuBezier(flame_obj_t *fo, unsigned x[4], unsigned y[4], unsigned x_max, unsigned y_max, unsigned d)
{
  unsigned r;
  double xt = 0.0, yt = 0.0;
  
  for (double t = 0.0; t <= 1.0; t += STEP)
    {
      xt = pow3(1 - t) * x[0] + 3 * t * pow2(1 - t) * x[1] + 3 * (1 - t) * pow2(t) * x[2] + pow3(t) * x[3];
      yt = pow3(1 - t) * y[0] + 3 * t * pow2(1 - t) * y[1] + 3 * (1 - t) * pow2(t) * y[2] + pow3(t) * y[3];
      
      flame_draw_point(fo, xt, yt);
    }
}

//
void roll_cuBezier(flame_obj_t *fo, unsigned x_min, unsigned x_max, unsigned y_min, unsigned y_max)
{
  unsigned  i = 0, r;
  unsigned X[NB_POINTS], Y[NB_POINTS];
  
  srand(time(NULL));
  
  for (int i = 0; i < NB_POINTS; i++)
    {
      X[i] = randxy(x_min, x_max);
      Y[i] = randxy(y_min, y_max);
    }
  
  while (i < MAX_CURVES)
    {
      cuBezier(fo, X, Y, x_max, y_max, DELAY);

      X[0] = X[3];                 Y[0] = Y[3];
      X[1] = randxy(x_min, x_max); Y[1] = randxy(x_min, x_max);
      X[2] = randxy(x_min, x_max); Y[2] = randxy(x_min, x_max);
      X[3] = randxy(x_min, x_max); Y[3] = randxy(x_min, x_max);
      
      i++;
    }
}

//
void collide_cuBezier(flame_obj_t *fo, unsigned x_min, unsigned x_max, unsigned y_min, unsigned y_max, unsigned ncurves)
{
  unsigned coll_x = 0, coll_y = 0;
  unsigned r, n = randxy(ncurves, ncurves << 2);
  unsigned XX[NB_POINTS * n], YY[NB_POINTS * n];
  unsigned X[NB_POINTS * ncurves], Y[NB_POINTS * ncurves];
    
  coll_x = randxy(x_min, x_max);
  coll_y = randxy(y_min, y_max);

  flame_set_color(fo, 255, 255, 255);
      
  for (unsigned i = 0; i < NB_POINTS * ncurves; i += 4)
    {

      X[i + 1] = randxy(x_min, x_max);
      Y[i + 1] = randxy(y_min, y_max);

      X[i + 2] = randxy(x_min, x_max);
      Y[i + 2] = randxy(y_min, y_max);

      X[i + 0] = randxy(x_min, x_max);
      Y[i + 0] = randxy(y_min, y_max);

      X[i + 3] = coll_x;
      Y[i + 3] = coll_y;

      cuBezier(fo, X + i, Y + i, x_max, y_max, DELAY);
    }

  for (unsigned i = 0; i < NB_POINTS * n; i += 4)
    {
      
      XX[i + 3] = randxy(x_min, x_max);
      YY[i + 3] = randxy(y_min, y_max);

      XX[i + 1] = randxy(x_min, x_max);
      YY[i + 1] = randxy(y_min, y_max);

      XX[i + 2] = randxy(x_min, x_max);
      YY[i + 2] = randxy(y_min, y_max);

      XX[i + 0] = coll_x;
      YY[i + 0] = coll_y;

      flame_set_color(fo, XX[i + 3] >> 3, XX[i + 3] >> 2, XX[i + 3] >> 1);
      cuBezier(fo, XX + i, YY + i, x_max, y_max, DELAY);
    }  
}

//
int main(int argc, char **argv)
{
  char c;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  unsigned X[NB_POINTS], Y[NB_POINTS];
  flame_obj_t *fo = flame_open("Bezier curve", x_max, y_max);

  flame_wait(fo, &click_x, &click_y);
  
 run:

  srand(time(NULL));
  
  for (int i = 0; i < NB_POINTS; i++)
    {
      X[i] = randxy(x_min, x_max);
      Y[i] = randxy(y_min, y_max);
    }
  
  flame_clear_display(fo);
  
  //cuBezier(fo, X, Y, x_max, y_max, DELAY);

  //roll_cuBezier(fo, x_min, x_max, y_min, y_max);

  collide_cuBezier(fo, x_min, x_max, y_min, y_max, 3);
  
  printf("Done\n");
  
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

  flame_close(fo);

  return 0;
}
