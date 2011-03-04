CC = gcc
CFLAGS = -pipe -m64 -Wall -O3 -mfpmath=sse -mtune=generic

BINNAME = sdl-flocking

all: sdl-flocking

sdl-flocking:
	$(CC) main.c boid.c vector.c $(CFLAGS) -lSDL -lSDL_image -o $(BINNAME) 

rebuild: clean sdl-flocking

clean:
	rm -rf $(BINNAME) *.o

.PHONY : clean
.SILENT : clean
