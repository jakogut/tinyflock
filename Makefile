CC = gcc
LD = gcc
CFLAGS = -pipe -Wall -O3 -mtune=generic
LDFLAGS = -lSDL_image `pkg-config --libs sdl`

BINNAME = sdl-flocking

OBJECTS = main.o vector.o boid.o

all: sdl-flocking

sdl-flocking: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(BINNAME) 

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

rebuild: clean sdl-flocking

clean:
	rm -rf $(BINNAME) *.o

.PHONY : clean
.SILENT : clean
