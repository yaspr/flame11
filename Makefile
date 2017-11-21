all:
	gcc -std=c99 -g3 flame.c mandelbrot.c -o mandelbrot -lm -lX11
