#ifndef FLOCK_H_
#define FLOCK_H_

#include "boid.h"
#include "configuration.h"

#define FLOCK_TYPE_RANDOM 0
#define FLOCK_TYPE_CENTERED 1

boid* create_flock(configuration* config);

void destroy_flock(boid* flock);

// Update the location and velocity of each boid in the flock
void flock_update(boid* flock, configuration* config);

// pthread args type for flock_render
typedef struct{ int run; boid* flock; configuration* config; SDL_Surface* screen; } flock_render_data;

// Render the flock (independent thread)
void* flock_render_pthread(void* arg);

static inline float rand_range(float min, float max);

#endif
