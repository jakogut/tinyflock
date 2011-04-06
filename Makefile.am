## Process this file with automake to generate Makefile.in
ACLOCAL_AMFLAGS = -I m4 --install
AM_CFLAGS = $(SDL_CFLAGS) $(SDL_image_CFLAGS)
LDADD = $(SDL_LIBS) $(SDL_image_LIBS)

noinst_PROGRAMS = flock
flock_SOURCES = boid.c boid.h configuration.h flock.c flock.h main.c vector.c vector.h
