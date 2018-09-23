#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include "flame.h"

#include "rdtsc.h"

#define MAX_STR 256

#define MAX_STALL_ITER 10000

#define MAX_ITER 1024 * 1024 * 2

#define UP    1
#define DOWN  3
#define LEFT  0
#define RIGHT 2

typedef unsigned char byte;

const char dir[4][6] = { "LEFT", "UP", "RIGHT", "DOWN" };

//Get the CPU budy to simulate velocity
void stall(unsigned n, unsigned *r)
{
  for (unsigned i = 0; i < n; i++)
    (*r)++;
}

//
byte randxy(byte x, byte y)
{ return (x + (rand() % y)) % y; }

/*
  Four directions
  
                      1
                      ^
                      |
               0 <--  x --> 2
                      |
                      v
                      3 

  Choose one and move accordingly. A random walk.
  Beautiful images emerge from this, something akin to continents.
  
 */
void walk(flame_obj_t *fo, unsigned x, unsigned y, unsigned x_min, unsigned x_max, unsigned y_min, unsigned y_max, int seed, FILE *fd)
{
  byte r;
  unsigned i = 0, px = x, py = y, _r_;
  
  srand(seed);
  
  while (i < MAX_ITER)
    {
      r = randxy(0, 4);

      fprintf(fd, "%s\n", dir[r]);
      
      if (r == UP)
	if (py > x_min)
	  py--;
	else
	  py++;
      else
	if (r == DOWN)
	  if (py < y_max)
	    py++;
	  else
	    py--;
	else
	  if (r == LEFT)
	    if (px > y_min)
	      px--;
	    else
	      px++;
	  else
	    if (r == RIGHT)
	      if (px < x_max)
		px++;
	      else
		px--;
      
      flame_draw_point(fo, px, py);
      
      //stall(MAX_STALL_ITER, &_r_);
      
      i++;
    }
}

//
int main(int argc, char **argv)
{
  char c, flog[MAX_STR];
  unsigned seed, reps = 0;
  int click_x, click_y;
  int x_min = 50, x_max = 1900;
  int y_min = 50, y_max = 1000; 
  FILE *fd = NULL, *fdl = NULL;
  flame_obj_t *fo = flame_open("Randwalk", x_max, y_max);
  
 run:

  fd = fopen("/dev/urandom", "rb");
  
  //Get a random seed from the system
  if (fd)
  {
    fread(&seed, sizeof(unsigned), 1, fd);
    fclose(fd);
  }

  sprintf(flog, "%u.randwalk.log", reps);

  fdl = fopen(flog, "wb");
  
  fprintf(fdl, "x(%u, %u):y(%u, %u)\n", x_min, x_max, y_min, y_max);
  
  flame_clear_display(fo);
  
  walk(fo, 300, 300, x_min, x_max, y_min, y_max, seed, fdl);
  
  printf("Done!\n");
  
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
	      reps++;
	      goto run; //No comment!
	    }
    }
  
  flame_close(fo);
  fclose(fdl);
  
  return 0;
}
