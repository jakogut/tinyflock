#ifndef FLOCK_H_
#define FLOCK_H_

#include "boid.h"

// Update the location and velocity of each boid in the flock
void flock_update(boid* flock, configuration* config);

// Render the flock
void flock_render(boid* flock, configuration* config, SDL_Surface* screen);

#endif
