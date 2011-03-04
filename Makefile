CC = gcc
CFLAGS = -pipe -m64 -Wall -O3 -mfpmath=sse -mtune=generic

BINNAME = swarm

all: swarm

swarm:
	$(CC) main.c boid.c vector.c $(CFLAGS) -lSDL -lSDL_image -lpthread -o $(BINNAME) 

rebuild: clean swarm

clean:
	rm -rf $(BINNAME) *.o

.PHONY : clean
.SILENT : clean
