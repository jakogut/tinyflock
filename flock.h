#ifndef FLOCK_H_
#define FLOCK_H_

#include "boid.h"

// Update the location and velocity of each boid in the flock
void flock_update(boid* flock, configuration* config);

// pthread args type for flock_render
typedef struct{ int run; boid* flock; configuration* config; SDL_Surface* screen; } flock_render_data;

// Render the flock (independent thread)
void* flock_render_pthread(void* arg);

#endif
