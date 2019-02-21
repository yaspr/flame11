all: k
	gcc -std=c99 -g3 flame.c mandelbrot.c -o mandelbrot -lm -lX11
	gcc -std=c99 -g3 flame.c randwalk.c   -o randwalk   -lm -lX11
	gcc -std=c99 -g3 flame.c curve.c      -o curve      -lm -lX11
	gcc -std=c99 -g3 flame.c lorenz.c     -o lorenz     -lm -lX11
	gcc -std=c99 -g3 flame.c barnsley.c   -o barnsley   -lm -lX11
	gcc -std=c99 -g3 flame.c ppm.c        -o ppm        -lm -lX11
	gcc -std=c99 -g3 flame.c lissajous.c  -o lissajous  -lm -lX11
	gcc -std=c99 -g3 flame.c perlin.c 3D.c  -o 3D  -lm -lX11

k:
	gcc -std=c99 -g3 -c flame.c display.c stack.c generator.c parser.c -lm -lX11
	gcc -std=c99 -g3 flame.o display.o stack.o generator.o parser.o keya.c -o keya -lm -lX11

	@rm *.o

clean:
	rm -rf *~ *.o mandelbrot randwalk curve lorenz barnsley keya 3D ppm lissajous
