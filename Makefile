all:
	gcc -std=c99 -g3 flame.c mandelbrot.c -o mandelbrot -lm -lX11
	gcc -std=c99 -g3 flame.c randwalk.c   -o randwalk   -lm -lX11
	gcc -std=c99  -g3  flame.c curve.c      -o curve      -lm -lX11

clean:
	rm -rf *~
