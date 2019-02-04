#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "pi.h"
#include "flame.h"
#include "perlin.h"

#include "rdtsc.h"

#define _Z_ 100

#define _S_ 10

//
typedef unsigned long long uint64;

//
typedef struct xy_s  { double x; double y; } xy_t; 
typedef struct xyz_s { double x; double y; double z; } xyz_t;

typedef struct object_3D_s { xyz_t *v; int **e; int n; int sqrt_n; } object_3D_t;

//
xyz_t *alloc_v_list(int n)
{
  return malloc(sizeof(xyz_t) * n);
}

//
void free_v_list(xyz_t *v)
{ if (v) free(v); }

//
int **alloc_e_matx(int n)
{
  int **tmp = malloc(sizeof(xyz_t *) * n);

  for (int i = 0; i < n; i++)
    {
      tmp[i] = malloc(sizeof(xyz_t) * n);
      memset(tmp[i], 0, n * sizeof(int));
    }
  
  return tmp;
}

//
void free_e_matx(int **e, int n)
{
  if (e)
    {
      for (int i = 1; i < n; i++)
	free(e[i]);
      
      free(e);
    }
}

//
void alloc_object_3D(int n, object_3D_t *o)
{
  o->n      = n;
  o->sqrt_n = sqrt(n);
  o->v      = alloc_v_list(n);
  o->e      = alloc_e_matx(n);;
}

//
void free_object_3D(object_3D_t *o)
{
  free_v_list(o->v);
  free_e_matx(o->e, o->n);
}

//
void rotate2D(double x, double y, double angle, double *ox, double *oy)
{
  *ox = -y * sin(angle) + x * cos(angle);
  *oy =  x * sin(angle) + y * cos(angle); 
}

//
void fill_triangle(flame_obj_t *fo, xy_t base, xyz_t v1, xyz_t v2, xyz_t v3, int r, int g, int b)
{
  float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
  float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);
  
  float curx1 = v1.x;
  float curx2 = v1.x;

  flame_set_color(fo, r, g, b);
  
  for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
    {
      flame_draw_line(fo, base.x - curx1, base.y + scanlineY, base.x - curx2, base.y + scanlineY);
      curx1 += invslope1;
      curx2 += invslope2;
    }

  flame_set_color(fo, 255, 255, 255);
}

//
void draw_vert(flame_obj_t *fo, xy_t base, xyz_t c)
{
  for (double a = 0.0; a < 2 * PI; a += 0.1)
    flame_draw_point(fo, base.x - (c.x + 2 * cos(a)), base.y + (c.y + 2 * sin(a)));
}

//
void render3D(flame_obj_t *fo, xy_t base, object_3D_t *o, xyz_t pov, xy_t rot, int edge_on, int point_on)
{
  xyz_t tmp;

  flame_set_color(fo, 255, 255, 255);
      
  //  
  for (int i = 0; i < o->n; i++)
    {
      tmp = o->v[i];
      
      tmp.x -= pov.x;
      tmp.y -= pov.y;
      tmp.z -= pov.z;
      
      rotate2D(tmp.x, tmp.z, rot.y, &tmp.x, &tmp.z);
      rotate2D(tmp.y, tmp.z, rot.x, &tmp.y, &tmp.z);
      
      o->v[i] = tmp;
      
      double f = _Z_ / o->v[i].z;
      
      o->v[i].x *= f;
      o->v[i].y *= f;


      if (point_on)
	flame_draw_point(fo, base.x - o->v[i].x, base.y + o->v[i].y);
      else
	draw_vert(fo, base, o->v[i]);
    }
  
  if (edge_on)
    {
      flame_set_color(fo, 169, 169, 169);
      
      //
      for (int i = 0; i < o->n; i++)
	{
	  for (int j = 0; j < i; j++)
	    if (o->e[i][j])
	      flame_draw_line(fo, base.x - o->v[i].x, base.y + o->v[i].y, base.x - o->v[j].x, base.y + o->v[j].y);
	  
	  for (int j = i + 1; j < o->n; j++)
	    if (o->e[i][j])
	      flame_draw_line(fo, base.x - o->v[i].x, base.y + o->v[i].y, base.x - o->v[j].x, base.y + o->v[j].y);
	}
      
      flame_set_color(fo, 255, 255, 255);    
    }
}

//
void draw_pyramid4(flame_obj_t *fo, object_3D_t *o, xy_t base, xyz_t pov, xy_t rot)
{
  o->v[0].x =  1; o->v[1].x = -1;  
  o->v[0].y =  1; o->v[1].y = -1;  
  o->v[0].z =  1; o->v[1].z =  1;  
  
  o->v[2].x = -1; o->v[3].x =  1;  
  o->v[2].y =  1; o->v[3].y = -1;  
  o->v[2].z = -1; o->v[3].z = -1;  
  
  for (int i = 0; i < o->n; i++)
    for (int j = 0; j < o->n; j++)
      o->e[i][j] = 0;
  
  o->e[0][1] = o->e[0][2] = o->e[0][3] = 1;
  o->e[1][0] = o->e[1][2] = o->e[1][3] = 1;
  o->e[2][0] = o->e[2][1] = o->e[2][3] = 1;
  o->e[3][0] = o->e[3][1] = o->e[3][2] = 1;
  
  render3D(fo, base, o, pov, rot, 1, 1);
}

//
void draw_pyramid5(flame_obj_t *fo, object_3D_t *o, xy_t base, xyz_t pov, xy_t rot)
{
  o->v[0].x =  0; o->v[1].x =  1;  
  o->v[0].y =  0; o->v[1].y =  0;  
  o->v[0].z = -1; o->v[1].z =  0;  
  
  o->v[2].x =  0; o->v[3].x = -1;  
  o->v[2].y =  0; o->v[3].y =  0;  
  o->v[2].z =  1; o->v[3].z =  0;  
  
  o->v[4].x = 0;
  o->v[4].y = -1;
  o->v[4].z = 0;
  
  for (int i = 0; i < o->n; i++)
    for (int j = 0; j < o->n; j++)
      o->e[i][j] = 0;
  
  //
  o->e[0][1] = o->e[0][4] = o->e[0][3] = 1;
  o->e[1][0] = o->e[1][4] = o->e[1][2] = 1;
  o->e[2][1] = o->e[2][4] = o->e[2][3] = 1;
  o->e[3][0] = o->e[3][4] = o->e[3][2] = 1;
  o->e[4][0] = o->e[4][1] = o->e[4][2] = o->e[4][3] = 1;
  
  render3D(fo, base, o, pov, rot, 1, 1);  
}

//
void draw_cube(flame_obj_t *fo, object_3D_t *o, xy_t base, xyz_t pov, xy_t rot)
{
  o->v[0].x = -1; o->v[1].x = -1;  
  o->v[0].y =  1; o->v[1].y = -1;  
  o->v[0].z = -1; o->v[1].z = -1;  
  
  o->v[2].x =  1; o->v[3].x =  1;  
  o->v[2].y = -1; o->v[3].y =  1;  
  o->v[2].z = -1; o->v[3].z = -1;  
  
  o->v[4].x = -1; o->v[5].x = -1;  
  o->v[4].y =  1; o->v[5].y = -1;  
  o->v[4].z =  1; o->v[5].z =  1;  
  
  o->v[6].x =  1; o->v[7].x =  1;  
  o->v[6].y = -1; o->v[7].y =  1;  
  o->v[6].z =  1; o->v[7].z =  1;  
  
  for (int i = 0; i < o->n; i++)
    for (int j = 0; j <o->n; j++)
      o->e[i][j] = 0;
  
  o->e[0][1] = o->e[0][4] = o->e[0][3] = 1;
  o->e[1][0] = o->e[1][5] = o->e[1][2] = 1;
  o->e[2][1] = o->e[2][6] = o->e[2][3] = 1;
  o->e[3][0] = o->e[3][7] = o->e[3][2] = 1;
  o->e[4][0] = o->e[4][5] = o->e[4][7] = 1;
  o->e[5][4] = o->e[5][6] = o->e[5][1] = 1;
  o->e[6][5] = o->e[6][2] = o->e[6][7] = 1;
  o->e[7][4] = o->e[7][3] = o->e[7][6] = 1;
  
  render3D(fo, base, o, pov, rot, 1, 1);
}

//
void draw_plane(flame_obj_t *fo, object_3D_t *o, xy_t base, xyz_t pov, xy_t rot)
{
  o->v[0].x = 0;
  o->v[0].y = 0;
  o->v[0].z = 0;
  
  o->v[1].x = 0;
  o->v[1].y = 1;
  o->v[1].z = 0;
  
  o->v[2].x = 0;
  o->v[2].y = 0;
  o->v[2].z = -1;
  
  o->v[3].x = 1;
  o->v[3].y = 0;
  o->v[3].z = 0;
  
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      o->e[i][j] = 0;
  
  o->e[0][1] = o->e[1][0] = 1;
  o->e[0][2] = o->e[2][0] = 1;
  o->e[0][3] = o->e[3][0] = 1;
  
  render3D(fo, base, o, pov, rot, 1, 1);
}

//
void init_terrain(object_3D_t *o)
{
  //Setting up edges
  for (int y = 0; y < o->n - o->sqrt_n; y++)
    {
      if ((y + 1) % o->sqrt_n)
	{
	  o->e[y][y + 1] = o->e[y][y + o->sqrt_n] = 1;
	  o->e[y + 1][y] = o->e[y + o->sqrt_n][y] = 1;
	  o->e[y + o->sqrt_n][y + 1] = o->e[y + 1][y + o->sqrt_n] = 1;
	}
      else
	o->e[y][y + o->sqrt_n] = o->e[y + o->sqrt_n][y] = 1;
    }
  
  for (int y = o->n - o->sqrt_n; y < o->n - 1; y++)
    o->e[y][y + 1] = o->e[y + 1][y] = 1;
}

//Generate y axis coordinates using Perlin noise (smoothness)
void draw_terrain(flame_obj_t *fo, object_3D_t *o, xy_t base, xyz_t pov, xy_t rot, int scale, int edge_on, int point_on)
{  
  for (int x = 0; x < o->sqrt_n; x++)
    for (int y = 0; y < o->sqrt_n; y++)
      {
  	o->v[x * o->sqrt_n + y].x = scale * x;
  	o->v[x * o->sqrt_n + y].z = y;
  	o->v[x * o->sqrt_n + y].y = scale * 20 * perlin2D((double)x, (double)y, 0.1, 20);
      }
  
  render3D(fo, base, o, pov, rot, edge_on, point_on);
}

//
int main(int argc, char **argv)
{
  char c, done = 0;
  int click_x, click_y;
  int x_min = 50, x_max = 1920;
  int y_min = 50, y_max = 1080;
  flame_obj_t *fo = flame_open("3D", x_max, y_max);
  
  srand(getpid());
  
  //
  xy_t base1, base2;
  
  base1.x = 300;
  base1.y = 600;
  
  base2.x = x_max / 2;
  base2.y = y_max / 2;
  
  xy_t rot;
  xyz_t pov;
  double dt = 0.1;
  
  //To avoid blow up reduce pov.z when objects are small
  pov.x = 460; pov.y = 600; pov.z = 230; //Best pov for terrain
  //pov.x = 10; pov.y = 10; pov.z = 10;
  rot.x = 0; rot.y = 0;
  
  object_3D_t plane;

  alloc_object_3D(4, &plane);
  
  object_3D_t cube;
  object_3D_t terrain;
  object_3D_t pyramid4;
  object_3D_t pyramid5;
  
  alloc_object_3D(8, &cube); 
  alloc_object_3D(10000, &terrain); init_terrain(&terrain);
  alloc_object_3D(4, &pyramid4);
  alloc_object_3D(5, &pyramid5);
  
  double after = 0.0, before = 0.0;

  int edge_mode = 1, point_mode = 1;

 run:

  //Beautiful things happen here :)
  
  flame_clear_display(fo);
  
  flame_set_color(fo, 0, 255, 0);
  
  draw_plane(fo, &plane, base1, pov, rot);
  
  flame_set_color(fo, 255, 255, 255);

  printf("POV: %lf %lf %lf\n", pov.x, pov.y, pov.z);
  
  //printf("Render cycles:\t%lf\n", (after - before));

  before = rdtsc();
  
  //draw_pyramid4(fo, &pyramid4, base2, pov, rot);
  
  draw_terrain(fo, &terrain, base2, pov, rot, _S_, edge_mode, point_mode);
  /* draw_cube(fo, &cube, base2, pov, rot); */

  after = rdtsc();
  
  //draw_pyramid4(fo, &pyramid4, base2, pov, rot);
  //draw_pyramid5(fo, &pyramid5, base2, pov, rot);
  
  //
  while (!done)
    {
      c = flame_wait(fo, &click_x, &click_y);
      
      if (c == 1)
	{
	  printf("Click at: %d, %d\n", click_x, click_y);

	  pov.x = click_x / 200;
	  pov.y = click_y / 200;
	  
	  rot.x += pov.x;
	  rot.y += pov.y;
	  
	  goto run;
	}
      else
	if (c == 'q')
	  done = 1;
      else
	if (c == 'a')
	  {
	    pov.x = 460; pov.y = 600; pov.z = 230;
	    //pov.x = 10; pov.y = 10; pov.z = 10;
	    rot.x = rot.y = 0;
	    
	    goto run;
	  }
	else
	  {
	    if (c == 'e')
	      {
		edge_mode = !edge_mode;

		goto run;
	      }
	    else
	      if (c == 'p')
		{
		  point_mode = !point_mode;
		  
		  goto run;
		}
	    
	    double s = dt * 10;
	    double _x_ = s * sin(rot.y), _y_ = s * cos(rot.y);
	    
	    //Up & Down
	    if (c == 'R') //Arrow up
	      {
		pov.y += s * _S_;
		goto run;
	      }
	    
	    if (c == 'T') //Arrow down
	      {
		pov.y -= s * _S_;
		goto run;
	      }
	    
	    //Forward & Backward
	    if (c == 'f')
	      {
		pov.z -= (pov.z > 2) ? _S_ * _y_ : 0;
		goto run;
	      }
	    
	    if (c == 'b')
	      {
		pov.z += _S_ * _y_ ;
		goto run;
	      }

	    //Right & Left
	    if (c == 'Q') //Arrow right
	      {	    
		pov.x -= _S_ * _y_; 
		goto run;
	      }
	    
	    if (c == 'S') //Arrow left
	      {
		pov.x += _S_ * _y_ ;
		goto run;
	      }
	  }
    }

  flame_close(fo);

  free_object_3D(&cube);
  free_object_3D(&terrain);
  free_object_3D(&pyramid4);
  free_object_3D(&pyramid5);
  
  return 0;
}
