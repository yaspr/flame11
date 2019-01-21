#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "pi.h"
#include "flame.h"
#include "stack.h"
#include "parser.h"
#include "display.h"
#include "generator.h"

//
int main(int argc, char **argv)
{
  if (argc < 4)
    return printf("OUPS: %s [input grammar file] [distance] [iterations]\n", argv[0]), -1;
  
  char c;
  display_t d;
  double r = 0.0;
  int click_x, click_y;
  int dist = atoi(argv[2]);
  int iter = atoi(argv[3]);
  int x_min = 50, x_max = 1920;
  int y_min = 50, y_max = 1080;
  parser_meta_t *p = load_file(argv[1]);
  flame_obj_t *fo = flame_open("Keya", x_max, y_max);

  if (!p)
    return printf("OUPS: Cannot find file '%s'\n", argv[1]), -2;
  
  print_parser_meta(p);

  click_x = x_max / 2;
  click_y = y_max / 2;

  srand(getpid());
  
 run:

  init_display(&d, fo, click_x, click_y, dist, x_max, y_max, -PI_2);

  draw_2D_plane(&d);
  
  generate1(&d, p, iter);
  
  while (1)
    {
      c = flame_wait(fo, &click_x, &click_y);

      if (c == 1)
	{
	  printf("Click at: %d, %d\n", click_x, click_y);

	  
	  goto run;
	}
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
