## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4 --install
AM_CFLAGS = $(SDL_CFLAGS) $(SDL_image_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
LDADD = $(SDL_LIBS) $(SDL_image_LIBS) $(GL_LIBS) $(GLU_LIBS)

noinst_PROGRAMS = flock
flock_SOURCES = boid.c boid.h configuration.h flock.c flock.h main.c vector.c vector.h
